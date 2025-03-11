// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactables/Rope.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Character/Logic/Base_MyCharacter.h"
#include "Controller/MyPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Data/Rope/RopeMovementSettings.h"
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
	DefaultLookPitch = RopeCamera->GetRelativeRotation().Pitch;
	HorizontalCameraOffset = FMath::Abs(GetActorLocation().Y - RopeCamera->GetComponentLocation().Y);
	VerticalCameraOffset = RopeCamera->GetComponentLocation().Z - GetActorLocation().Z;
	CurrCharacterAttachHorizontalAngle = 0;

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
void ARope::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if (IsOccupied)
	{
		UpdateCameraPositionAndRotation();
		AdjustHorizontalPosition(deltaTime);
				
		FName visualBoneName = FName(GetBoneName(AttachedBone + RopeSettings->NumberOfBonesToOffsetGrip));
		
		FVector inRopeSpaceLeft = FRotationMatrix(Rope->GetSocketRotation(visualBoneName)).TransformVector(RopeSettings->LeftHandGripOffsetDirection.RotateAngleAxis(CurrCharacterAttachHorizontalAngle, {0, 0, 1}));
		FVector leftHandPosition = Rope->GetSocketLocation(visualBoneName) + inRopeSpaceLeft * RopeSettings->LeftHandGripOffsetAmount;
		
		FVector inRopeSpaceRight = FRotationMatrix(Rope->GetSocketRotation(visualBoneName)).TransformVector(RopeSettings->RightHandGripOffsetDirection.RotateAngleAxis(CurrCharacterAttachHorizontalAngle, { 0, 0, 1 }));
		FVector rightHandPosition = Rope->GetSocketLocation(visualBoneName) + inRopeSpaceRight * RopeSettings->RightHandGripOffsetAmount;

		AttachedCharacter->UpdateHandPositions(leftHandPosition, rightHandPosition);
	}
}
#pragma endregion

#pragma region Camera
void ARope::UpdateCameraPositionAndRotation()
{
	FVector cameraWorldLocation = GetActorLocation() + (-FVector::ForwardVector * HorizontalCameraOffset).RotateAngleAxis(TargetCharacterAttachHorizontalAngle, FVector::UpVector) + FVector{0, 0, VerticalCameraOffset};
	RopeCamera->SetWorldLocation(cameraWorldLocation);

	FQuat cameraWorldRotation = FQuat::MakeFromRotator(FRotator{ DefaultLookPitch, TargetCharacterAttachHorizontalAngle, 0 });
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

	InputState = ERopeInputState::SWING;
	AttachedBoneName = attachBone;
	AttachedBone = GetBoneNumber(AttachedBoneName.ToString());
	VisualAttachedBoneName = FName(GetBoneName(AttachedBone + RopeSettings->NumberOfBonesToOffsetGrip));

	IsOccupied = true;
	HasJumpInputStarted = false;
	
	AttachedCharacter = character;
	AttachedCharacter->AttachToComponent(this->Rope, { EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true }, AttachedBoneName);

	CurrCharacterAttachHorizontalAngle = AttachedCharacter->GetCurrentState().CurrCharacterRotation;
	TargetCharacterAttachHorizontalAngle = CurrCharacterAttachHorizontalAngle;

	RopeCamera->SetActive(true);
	RopeAttachEvent.Broadcast(this);

	DeactivateCollision();

	SubscribeToInput();

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

	UnsubscribeToInput();

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
void ARope::SubscribeToInput()
{
	TObjectPtr<AMyPlayerController> myPlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT).ActionTriggeredEvent.AddUObject(this, &ARope::OnMovementInputTriggered);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT).ActionCompletedEvent.AddUObject(this, &ARope::OnMovementInputCompleted);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionTriggeredEvent.AddUObject(this, &ARope::OnReleaseTriggered);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionStartedEvent.AddUObject(this, &ARope::StartRelease);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_GAIT).ActionStartedEvent.AddUObject(this, &ARope::OnToggleControls);
}

void ARope::UnsubscribeToInput()
{
	TObjectPtr<AMyPlayerController> myPlayerController = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT).ActionTriggeredEvent.RemoveAll(this);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT).ActionCompletedEvent.RemoveAll(this);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionTriggeredEvent.RemoveAll(this);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionStartedEvent.RemoveAll(this);
	myPlayerController->GetActionInputWrapper(FInputType::LOCOMOTION_GAIT).ActionStartedEvent.RemoveAll(this);
}

void ARope::StartRelease(const FInputActionInstance& input)
{
	HasJumpInputStarted = true;
}

void ARope::OnMovementInputTriggered(const FInputActionInstance& inputActionInstance)
{
	FVector2D movementInput = inputActionInstance.GetValue().Get<FVector2D>();
	movementInput.Normalize();

	switch (InputState)
	{
		case ERopeInputState::SHIMMY:
			OnShimmy(movementInput);
			break;
		case ERopeInputState::SWING:
			OnSwing(movementInput);
			break;
	}
}

void ARope::OnMovementInputCompleted(const FInputActionInstance& inputActionInstance) const
{
	AttachedCharacter->UpdateSwingInput(FVector2D::Zero());
}

void ARope::OnReleaseTriggered(const FInputActionInstance& inputActionInstance)
{
	if (!HasJumpInputStarted)
		return;
	
	TryDetachCharacter();
}

void ARope::OnToggleControls(const FInputActionInstance& inputActionInstance)
{
	InputState = InputState == ERopeInputState::SHIMMY ? ERopeInputState::SWING : ERopeInputState::SHIMMY;
	RopeStateToggleEvent.Broadcast(InputState);
}
#pragma endregion

#pragma region Swing
void ARope::OnSwing(FVector2D normalizedMovementInput) const
{
	normalizedMovementInput = FVector2D{ normalizedMovementInput.Y, normalizedMovementInput.X };
	FVector2D rotatedMovementInput = normalizedMovementInput.GetRotated(CurrCharacterAttachHorizontalAngle);
	Rope->AddForce({ rotatedMovementInput.X * RopeSettings->RopeForce, rotatedMovementInput.Y * RopeSettings->RopeForce, 0 }, BoneToApplyForce);
	AttachedCharacter->UpdateSwingInput(normalizedMovementInput);
}
#pragma endregion

#pragma region Shimmy
void ARope::OnShimmy(FVector2D normalizedMovementInput)
{
	TargetCharacterAttachHorizontalAngle = FMath::ClampAngle(TargetCharacterAttachHorizontalAngle - normalizedMovementInput.X * RopeMovementSettings->CameraHorizontalSpeed, -180, 179.9);
}

void ARope::AdjustHorizontalPosition(float deltaTime)
{
	CurrCharacterAttachHorizontalAngle = FMath::FInterpTo(CurrCharacterAttachHorizontalAngle, TargetCharacterAttachHorizontalAngle, deltaTime, RopeMovementSettings->CharacterHorizontalShimmySpeed);
	FRotator visualBoneRotation{Rope->GetSocketRotation(VisualAttachedBoneName)};
	FVector boneUpVector = FRotationMatrix(visualBoneRotation).GetScaledAxis(EAxis::Y);
	FVector boneForwardVector = FRotationMatrix(visualBoneRotation).GetScaledAxis(EAxis::Z);

	FVector attachedBoneLocation = Rope->GetSocketLocation(AttachedBoneName);
	FVector visualBoneLocation = Rope->GetSocketLocation(VisualAttachedBoneName);
	FVector characterOffsetDirection = boneForwardVector.RotateAngleAxis(CurrCharacterAttachHorizontalAngle, boneUpVector);
	AttachedCharacter->SetActorLocation(attachedBoneLocation + characterOffsetDirection * RopeSettings->CharacterOffset);

	AttachedCharacter->SetActorRotation(UKismetMathLibrary::FindLookAtRotation(AttachedCharacter->GetActorLocation(), attachedBoneLocation));
}
#pragma endregion

#pragma region State
FRopeStateToggleDelegate ARope::RopeStateToggleEvent;

void ARope::ResetSwingableState()
{
	CanSwing = true;
	ActivateCollision();
}
#pragma endregion
