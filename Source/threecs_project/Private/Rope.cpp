// Fill out your copyright notice in the Description page of Project Settings.

#include "Rope.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Character/Logic/Base_MyCharacter.h"
#include "Controller/MyPlayerController.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region Initialisation
// Sets default values
ARope::ARope()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RopeTop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rope top"));
	RootComponent = RopeTop;

	Rope = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Rope"));
	Rope->SetupAttachment(RootComponent);

	// TODO: Spawn more spheres along the length to represent each bone
	// TODO: Scale the length of the rope (maybe through hiding bones or through a spline mesh)
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collider"));
	SphereCollision->InitSphereRadius(SphereCollisionRadius);
	SphereCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	SphereCollision->SetupAttachment(Rope, FName{"Bone_001"});
	SphereCollision->bHiddenInGame = false;

	// camera for better viewing of the character
	// TODO: Adjust positioning based on character position
	RopeCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	RopeCamera->SetupAttachment(RootComponent);
	RopeCamera->bAutoActivate = false;

	SwingCooldown = 10;
	RopeForce = 30000;
	CameraOffset = 300;
	AttachBone = FName{ "Bone_022" };
	SimulatePhysicsBone = FName{ "Bone_100" };
	BoneToApplyForce = FName{ "Bone_050" };
}

// Called when the game starts or when spawned
void ARope::BeginPlay()
{
	Super::BeginPlay();

	HasJumpInputStarted = false;
	CanSwing = true;
	IsOccupied = false;
	DefaultLookPitch = RopeCamera->GetComponentRotation().Pitch;
	CharacterAttachHorizontalAngle = 0;

	UpdateCameraPositionAndRotation();

	this->Rope->SetAllBodiesBelowSimulatePhysics(SimulatePhysicsBone, true);
	
	ActivateCollision();
}

void ARope::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DeactivateCollision();
}

// Called every frame
void ARope::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawDebugString(GetWorld(), Rope->GetSocketLocation(SimulatePhysicsBone), SimulatePhysicsBone.ToString());

	if (IsOccupied)
	{
		UpdateCameraPositionAndRotation();
	}
}
#pragma endregion

#pragma region Camera
void ARope::UpdateCameraPositionAndRotation()
{
	FVector cameraWorldLocation = RootComponent->GetComponentLocation() + (-FVector::ForwardVector * CameraOffset).RotateAngleAxis(CharacterAttachHorizontalAngle, FVector::UpVector);
	RopeCamera->SetWorldLocation(cameraWorldLocation);

	FQuat cameraWorldRotation = FQuat::MakeFromRotator(FRotator{ DefaultLookPitch, CharacterAttachHorizontalAngle, 0 });
	RopeCamera->SetWorldRotation(cameraWorldRotation);
}
#pragma endregion

#pragma region Collision
void ARope::OnCharacterOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABase_MyCharacter* character = Cast<ABase_MyCharacter>(OtherActor);
	if (!character)
		return;
	
	TryAttachCharacter(character);
}
#pragma endregion

#pragma region Attach
FRopeAttachDelegate ARope::RopeAttachEvent;

bool ARope::TryAttachCharacter(ABase_MyCharacter* character)
{
	if (!CanSwing)
		return false;
	if (IsOccupied)
		return false;

	FVector currCharacterVelocity = character->GetMovementComponent()->Velocity;
	currCharacterVelocity.Z = 0;

	if (!character->EnterSwingState())
		return false;

	IsOccupied = true;
	HasJumpInputStarted = false;

	AttachedCharacter = character;
	AttachedCharacter->AttachToComponent(this->Rope, { EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true }, AttachBone);
	CharacterAttachHorizontalAngle = AttachedCharacter->GetCurrentState().CurrCharacterRotation;

	RopeCamera->SetActive(true);
	RopeAttachEvent.Broadcast(this);

	DeactivateCollision();

	SubscribeToMovement();

	Rope->SetPhysicsLinearVelocity(currCharacterVelocity, true, BoneToApplyForce);
	return true;
}
#pragma endregion

#pragma region Detach
FRopeDetachDelegate ARope::RopeDetachEvent;

bool ARope::TryDetachCharacter()
{
	if (!IsOccupied)
		return false;
	if (!AttachedCharacter)
		return false;

	IsOccupied = false;

	CanSwing = false;
	GetWorld()->GetTimerManager().SetTimer(ResetSwingStateTimer, this, &ARope::ResetSwingableState, SwingCooldown, false);
	
	AttachedCharacter->ExitSwingState();
	AttachedCharacter->DetachFromActor({ EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, EDetachmentRule::KeepWorld, false });
	FVector currRopeVelocity = Rope->GetPhysicsLinearVelocity(BoneToApplyForce);
	currRopeVelocity.Z = 0;
	AttachedCharacter->LaunchCharacter(currRopeVelocity, true, true);
	
	RopeDetachEvent.Broadcast();

	AttachedCharacter = nullptr;
	HasJumpInputStarted = false;
	
	return true;
}
#pragma endregion

#pragma region Input
void ARope::SubscribeToMovement()
{
	TObjectPtr<AMyPlayerController> myPlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT).ActionTriggeredEvent.AddUObject(this, &ARope::OnMovementTriggered);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionTriggeredEvent.AddUObject(this, &ARope::OnJumpTriggered);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionStartedEvent.AddUObject(this, &ARope::StartJump);
}

void ARope::UnsubcribeToMovement()
{
	TObjectPtr<AMyPlayerController> myPlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT).ActionTriggeredEvent.RemoveAll(this);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionTriggeredEvent.RemoveAll(this);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionStartedEvent.RemoveAll(this);
}

void ARope::StartJump(const FInputActionInstance& input)
{
	HasJumpInputStarted = true;
}

void ARope::OnMovementTriggered(const FInputActionInstance& inputActionInstance) const
{
	FVector2D movementInput = inputActionInstance.GetValue().Get<FVector2D>();
	movementInput = FVector2D{ movementInput.Y, movementInput.X };
	movementInput.Normalize();
	FVector2D rotatedMovementInput = movementInput.GetRotated(CharacterAttachHorizontalAngle);
	Rope->AddForce({ rotatedMovementInput.X * RopeForce, rotatedMovementInput.Y * RopeForce, 0 }, BoneToApplyForce);
	AttachedCharacter->UpdateSwingInput(movementInput);
}

void ARope::OnJumpTriggered(const FInputActionInstance& inputActionInstance)
{
	if (!HasJumpInputStarted)
		return;
	
	TryDetachCharacter();
}
#pragma endregion

#pragma region State
void ARope::ResetSwingableState()
{
	CanSwing = true;
	ActivateCollision();
}
#pragma endregion
