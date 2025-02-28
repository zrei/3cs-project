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

	// camera for better viewing of the character
	RopeCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	RopeCamera->SetupAttachment(RootComponent);
	RopeCamera->bAutoActivate = false;
}

void ARope::OnConstruction(const FTransform& Transform)
{
	if (!RopeSettings)
		return;

	if (Rope)
	{	
		for (unsigned int i = 0; i < RopeSettings->NumberOfGrabbableBonesFromTheBottom; ++i)
		{
			unsigned int boneNumber = TOTAL_BONES - RopeSettings->VisibleBones + i;
			TObjectPtr<USphereComponent> sphereCollision = Cast<USphereComponent>(AddComponentByClass(USphereComponent::StaticClass(), true, {}, true));
			sphereCollision->SetupAttachment(Rope, FName(GetBoneName(boneNumber + 1)));
			sphereCollision->InitSphereRadius(RopeSettings->SphereColliderRadius);
			sphereCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
			sphereCollision->bHiddenInGame = false;
		}
	}
}

// Called when the game starts or when spawned
void ARope::BeginPlay()
{
	Super::BeginPlay();

	HasJumpInputStarted = false;
	CanSwing = true;
	IsOccupied = false;
	DefaultLookPitch = RopeCamera->GetComponentRotation().Pitch;
	CameraOffset = FVector::Dist(RopeCamera->GetComponentLocation(), GetActorLocation());
	CharacterAttachHorizontalAngle = 0;

	UpdateCameraPositionAndRotation();

	BoneToSimulatePhysics = FName(GetBoneName(TOTAL_BONES - RopeSettings->VisibleBones + RopeSettings->BoneFromTheBottomToSimulatePhysics));
	this->Rope->SetAllBodiesBelowSimulatePhysics(BoneToSimulatePhysics, true);
	BoneToApplyForce = FName(GetBoneName(TOTAL_BONES - RopeSettings->VisibleBones + RopeSettings->BoneFromTheBottomToApplyForce));

	ActivateCollision();

	for (unsigned int i = 0; i < TOTAL_BONES - RopeSettings->VisibleBones; ++i)
	{
		Rope->HideBoneByName(FName(GetBoneName(i + 1)), PBO_Term);
	}
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

	DrawDebugString(GetWorld(), Rope->GetSocketLocation(BoneToSimulatePhysics), BoneToSimulatePhysics.ToString() + FString(": Simulate physics"));
	DrawDebugString(GetWorld(), Rope->GetSocketLocation(BoneToApplyForce), BoneToApplyForce.ToString() + FString(": Apply force"));

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
	
	TryAttachCharacter(character, OverlappedComp->GetAttachSocketName());
}
#pragma endregion

#pragma region Attach
FRopeAttachDelegate ARope::RopeAttachEvent;

bool ARope::TryAttachCharacter(ABase_MyCharacter* character, const FName& attachBone)
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
	AttachedCharacter->AttachToComponent(this->Rope, { EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true }, attachBone);
	AttachedCharacter->AddActorLocalOffset({ -RopeSettings->CharacterOffset, 0, 0 });
	CharacterAttachHorizontalAngle = AttachedCharacter->GetCurrentState().CurrCharacterRotation;

	RopeCamera->SetActive(true);
	RopeAttachEvent.Broadcast(this);

	DeactivateCollision();

	SubscribeToMovement();

	Rope->SetPhysicsLinearVelocity(currCharacterVelocity * InitialVelocityMultiplier, true, BoneToApplyForce);
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

	UnsubscribeToMovement();

	IsOccupied = false;

	CanSwing = false;
	GetWorld()->GetTimerManager().SetTimer(ResetSwingStateTimer, this, &ARope::ResetSwingableState, RopeSettings->SwingCooldown, false);
	
	AttachedCharacter->DetachFromActor({ EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false });
	AttachedCharacter->ExitSwingState();
	FVector currRopeVelocity = Rope->GetPhysicsLinearVelocity(BoneToApplyForce);
	currRopeVelocity.Z = 0;

	AttachedCharacter->LaunchCharacter(currRopeVelocity * LaunchVelocityMultiplier, true, true);
	
	RopeCamera->SetActive(false);
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
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT).ActionCompletedEvent.AddUObject(this, &ARope::OnMovementCompleted);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionTriggeredEvent.AddUObject(this, &ARope::OnJumpTriggered);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionStartedEvent.AddUObject(this, &ARope::StartJump);
}

void ARope::UnsubscribeToMovement()
{
	TObjectPtr<AMyPlayerController> myPlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT).ActionTriggeredEvent.RemoveAll(this);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT).ActionCompletedEvent.RemoveAll(this);
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
	Rope->AddForce({ rotatedMovementInput.X * RopeSettings->RopeForce, rotatedMovementInput.Y * RopeSettings->RopeForce, 0 }, BoneToApplyForce);
	AttachedCharacter->UpdateSwingInput(movementInput);
}

void ARope::OnMovementCompleted(const FInputActionInstance& inputActionInstance) const
{
	AttachedCharacter->UpdateSwingInput(FVector2D::Zero());
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
