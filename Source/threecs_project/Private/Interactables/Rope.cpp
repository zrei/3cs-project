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
#include "Camera/MyPlayerCameraManager.h"

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
	Super::OnConstruction(Transform);

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
			sphereCollision->bHiddenInGame = true;
		}
	}
}

// Called when the game starts or when spawned
void ARope::BeginPlay()
{
	Super::BeginPlay();

	CanSwing = true;
	IsOccupied = false;
	HorizontalCameraOffset = FMath::Abs(GetActorLocation().Y - RopeCamera->GetComponentLocation().Y);
	VerticalCameraOffset = RopeCamera->GetComponentLocation().Z - GetActorLocation().Z;
	CurrCharacterAttachRotation = FRotator{0};
	TargetCharacterAttachRotation = FRotator{0};
	CameraRotation = FRotator{ RopeCamera->GetRelativeRotation().Pitch, CurrCharacterAttachRotation.Yaw, 0};;

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
		CheckCharacterInputs(deltaTime);

		if (!AttachedCharacter)
			return;

		UpdateCameraPositionAndRotation();
		AdjustHorizontalPosition(deltaTime);
				
		FName visualBoneName = FName(GetBoneName(AttachedBone + RopeSettings->NumberOfBonesToOffsetGrip));
		
		FVector leftHandPosition = Rope->GetSocketLocation(visualBoneName);
		
		FVector rightHandPosition = Rope->GetSocketLocation(visualBoneName);

		AttachedCharacter->UpdateHandPositions(leftHandPosition, rightHandPosition);
	}
}
#pragma endregion

#pragma region Camera
void ARope::UpdateCameraPositionAndRotation()
{
	FVector cameraWorldLocation = GetActorLocation() + (-FVector::ForwardVector * HorizontalCameraOffset).RotateAngleAxis(CameraRotation.Yaw, FVector::UpVector) + FVector{0, 0, VerticalCameraOffset};
	RopeCamera->SetWorldLocation(cameraWorldLocation);
	FQuat cameraWorldRotation = FQuat::MakeFromRotator(CameraRotation);
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

	AttachedBoneName = attachBone;
	AttachedBone = GetBoneNumber(AttachedBoneName.ToString());
	VisualAttachedBoneName = FName(GetBoneName(AttachedBone + RopeSettings->NumberOfBonesToOffsetGrip));

	IsOccupied = true;
	
	AttachedCharacter = character;
	AttachedCharacter->AttachToComponent(this->Rope, { EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true }, AttachedBoneName);

	CurrCharacterAttachRotation = AttachedCharacter->GetCurrentState().CurrCharacterRotation;
	TargetCharacterAttachRotation = CurrCharacterAttachRotation;
	CameraRotation.Yaw = CurrCharacterAttachRotation.Yaw;

	RopeCamera->SetActive(true);
	UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->SetViewTarget(this, RopeSettings->TransitionInParams);

	DeactivateCollision();

	Rope->SetPhysicsLinearVelocity(currCharacterVelocity * RopeSettings->InitialVelocityMultiplier, true, BoneToApplyForce);
	return true;
}
#pragma endregion

#pragma region Detach
bool ARope::TryDetachCharacter()
{
	if (!IsOccupied)
		return false;
	if (!AttachedCharacter)
		return false;

	IsOccupied = false;

	CanSwing = false;
	GetWorld()->GetTimerManager().SetTimer(ResetSwingStateTimer, this, &ARope::ResetSwingableState, RopeSettings->SwingCooldown, false);
	
	AttachedCharacter->DetachFromActor({ EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false });
	AttachedCharacter->ExitSwingState();
	FVector currRopeVelocity = Rope->GetPhysicsLinearVelocity(BoneToApplyForce);
	currRopeVelocity.Z = 0;

	AttachedCharacter->LaunchCharacter(currRopeVelocity * RopeSettings->LaunchVelocityMultiplier, true, true);
	
	RopeCamera->SetActive(false);
	UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->SetViewTarget(Cast<AActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)), RopeSettings->TransitionOutParams);

	AttachedCharacter = nullptr;
	
	return true;
}
#pragma endregion

#pragma region Input
void ARope::CheckCharacterInputs(float deltaTime)
{
	if (AttachedCharacter->GetCurrentState().CharacterMovementState == ECharacterMovementState::EXIT_SWINGING)
	{
		TryDetachCharacter();
		return;
	}

	FVector2D movementInput = AttachedCharacter->GetCurrentState().MovementInput;

	switch (AttachedCharacter->GetCurrentState().RopeInputState)
	{
		case ERopeInputState::SHIMMY:
			OnShimmy(movementInput, deltaTime);
			break;
		case ERopeInputState::SWING:
			OnSwing(movementInput, deltaTime);
			break;
	}
}
#pragma endregion

#pragma region Swing
void ARope::OnSwing(FVector2D normalizedMovementInput, float deltaTime) const
{
	normalizedMovementInput = FVector2D{ normalizedMovementInput.Y, normalizedMovementInput.X };
	FVector2D rotatedMovementInput = normalizedMovementInput.GetRotated(CurrCharacterAttachRotation.Yaw);
	Rope->AddForce({ rotatedMovementInput.X * RopeSettings->RopeForce * deltaTime, rotatedMovementInput.Y * RopeSettings->RopeForce * deltaTime, 0 }, BoneToApplyForce);
}
#pragma endregion

#pragma region Shimmy
void ARope::OnShimmy(FVector2D normalizedMovementInput, float deltaTime)
{
	CameraRotation.Yaw = FMath::ClampAngle(CameraRotation.Yaw - normalizedMovementInput.X * RopeMovementSettings->CameraHorizontalSpeed * deltaTime, -180, 179.9);
	TargetCharacterAttachRotation = FRotator{0, CameraRotation.Yaw, 0};
}

void ARope::AdjustHorizontalPosition(float deltaTime)
{
	CurrCharacterAttachRotation = UKismetMathLibrary::RInterpTo(CurrCharacterAttachRotation, TargetCharacterAttachRotation, deltaTime, RopeMovementSettings->CharacterHorizontalShimmySpeed);

	FRotator visualBoneRotation{Rope->GetSocketRotation(VisualAttachedBoneName)};
	FVector boneUpVector = FRotationMatrix(visualBoneRotation).GetScaledAxis(EAxis::Y);
	FVector boneForwardVector = FRotationMatrix(visualBoneRotation).GetScaledAxis(EAxis::Z);

	FVector attachedBoneLocation = Rope->GetSocketLocation(AttachedBoneName);
	FVector visualBoneLocation = Rope->GetSocketLocation(VisualAttachedBoneName);
	FVector characterOffsetDirection = boneForwardVector.RotateAngleAxis(CurrCharacterAttachRotation.Yaw, boneUpVector);
	AttachedCharacter->SetActorLocation(attachedBoneLocation + characterOffsetDirection * RopeSettings->CharacterOffset);

	AttachedCharacter->SetActorRotation(UKismetMathLibrary::FindLookAtRotation(AttachedCharacter->GetActorLocation(), attachedBoneLocation));
}
#pragma endregion

#pragma region State
void ARope::ResetSwingableState()
{
	CanSwing = true;
	ActivateCollision();
}
#pragma endregion
