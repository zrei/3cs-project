// Fill out your copyright notice in the Description page of Project Settings.


#include "Rope.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Character/Logic/Base_MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Controller/MyPlayerController.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"

// Sets default values
ARope::ARope()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RopeTop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rope top"));
	RootComponent = RopeTop;

	Rope = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Rope"));
	Rope->SetupAttachment(RootComponent);

	// parent the sphere to the cable and attach it to the cable, specifically to cable end
	// on attach we need to set constraint components 2 to be the character (mesh?) and hand_r
	// the cable needs to set attach end to true and attach end to the hand_rSocket on the character mesh
	// drop out when jumping, should NOT reattach until you re-enter
	// will have to see how it plays with the MOVE input...
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collider"));
	SphereCollision->InitSphereRadius(20);
	SphereCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	SphereCollision->SetupAttachment(Rope, FName{"Bone_001"});
	SphereCollision->bHiddenInGame = false;

	RopeCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	RopeCamera->SetupAttachment(RootComponent);
	RopeCamera->bAutoActivate = false;

	SwingCooldown = 10;
}

// Called when the game starts or when spawned
void ARope::BeginPlay()
{
	Super::BeginPlay();

	CanSwing = true;

	this->Rope->SetAllBodiesBelowSimulatePhysics(FName{ "Bone_100" }, true);
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ARope::OnCharacterOverlap);
	IsOccupied = false;

	DefaultLookPitch = RopeCamera->GetComponentRotation().Pitch;
	RopeCamera->SetWorldLocation(RootComponent->GetComponentLocation() - FVector::ForwardVector * CameraOffset);
	RopeCamera->SetWorldRotation(FQuat::MakeFromRotator(FRotator{ DefaultLookPitch, CharacterHorizontalAngle, 0 }));
}

void ARope::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	SphereCollision->OnComponentBeginOverlap.RemoveDynamic(this, &ARope::OnCharacterOverlap);
}

// Called every frame
void ARope::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawDebugString(GetWorld(), Rope->GetSocketLocation(FName{ "Bone_100" }), FString{ "Bone_100" });

	if (IsOccupied)
	{
		FVector cameraWorldLocation = RootComponent->GetComponentLocation() + (-FVector::ForwardVector * CameraOffset).RotateAngleAxis(CharacterHorizontalAngle, FVector::UpVector);
		RopeCamera->SetWorldLocation(cameraWorldLocation);

		FQuat cameraWorldRotation = FQuat::MakeFromRotator(FRotator{ DefaultLookPitch, CharacterHorizontalAngle, 0 });
		RopeCamera->SetWorldRotation(cameraWorldRotation);
	}
}

void ARope::OnCharacterOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!CanSwing)
		return;
	UE_LOG(LogTemp, Warning, TEXT("OVERLAP!!!"));
	if (IsOccupied)
		return;
	ABase_MyCharacter* character = Cast<ABase_MyCharacter>(OtherActor);
	if (!character)
		return;
	if (!character->EnterSwingState())
		return;
	IsOccupied = true;
	character->GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;
	character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	character->GetCharacterMovement()->StopMovementImmediately();

	character->AttachToComponent(this->Rope, {EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true}, FName{"Bone_022"});
	SubscribeToMovement();
	RopeCamera->SetActive(true);
	RopeAttachEvent.Broadcast(this);

	CharacterHorizontalAngle = character->GetCurrentState().CurrCharacterRotation;

	SphereCollision->OnComponentBeginOverlap.RemoveAll(this);

	AttachedCharacter = character;
}

void ARope::SubscribeToMovement()
{
	TObjectPtr<AMyPlayerController> myPlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT).ActionTriggeredEvent.AddUObject(this, &ARope::OnMovementTriggered);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionTriggeredEvent.AddUObject(this, &ARope::ReleaseRope);
}

void ARope::UnsubcribeToMovement()
{
	TObjectPtr<AMyPlayerController> myPlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT).ActionTriggeredEvent.RemoveAll(this);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionTriggeredEvent.RemoveAll(this);
}

void ARope::OnMovementTriggered(const FInputActionInstance& inputActionInstance)
{
	FVector2D movementInput = inputActionInstance.GetValue().Get<FVector2D>();
	movementInput.Normalize();
	UE_LOG(LogTemp, Warning, TEXT("Movement: %f, %f"), movementInput.X, movementInput.Y);
	Rope->AddForce({ movementInput.X * 30000, movementInput.Y * 30000, 0 }, FName{ "Bone_050" });
}

void ARope::ReleaseRope(const FInputActionInstance& inputActionInstance)
{
	if (!IsOccupied)
		return;
	if (!AttachedCharacter)
		return;
	IsOccupied = false;
	CanSwing = false;
	FTimerHandle throwaway;
	GetWorld()->GetTimerManager().SetTimer(throwaway, this, &ARope::ResetSwingableState, SwingCooldown, false);
	AttachedCharacter->ExitSwingState();
	AttachedCharacter->DetachFromActor({EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, EDetachmentRule::KeepWorld, false});
	AttachedCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AttachedCharacter->GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Falling;
	RopeDetachEvent.Broadcast();

	AttachedCharacter = nullptr;
}

void ARope::ResetSwingableState()
{
	CanSwing = true;
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ARope::OnCharacterOverlap);
}

FRopeAttachDelegate ARope::RopeAttachEvent;
FRopeDetachDelegate ARope::RopeDetachEvent;