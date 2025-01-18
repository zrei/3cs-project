// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Logic/Base_MyCharacter.h"
#include "Controller/MyPlayerController.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"

#pragma region Initialisation
// Sets default values
ABase_MyCharacter::ABase_MyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;

	CameraParent = CreateDefaultSubobject<USceneComponent>(TEXT("Camera Parent"));
	CameraParent->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraParent);

	MovementSettings.CharacterAcceleration = 0.2;
	MovementSettings.CharacterDecceleration = 0.3;
	MovementSettings.CharacterRunMovementSpeed = 0.7;
	MovementSettings.CharacterWalkMovementSpeed = 0.3;
	MovementSettings.CharacterStationaryRotationalSpeed = 15;
	MovementSettings.CharacterMovingRotationalSpeed = 70;
	MovementSettings.MovingRotationTime = 0.5;
	MovementSettings.RotationAngleThreshold = 45;
	MovementSettings.RotationLookTimeThreshold = 1;

	CameraSettings.CameraRotationalSpeed = 10;
	CameraSettings.MaxViewVerticalAngle = 30;
	CameraSettings.MinViewVerticalAngle = -60;

	RotationCurveName = FName{ "RotationAmount" };
	LegsSlotName = FName{ "Legs" };
}

// Called when the game starts or when spawned
void ABase_MyCharacter::BeginPlay()
{
	Super::BeginPlay();

	TObjectPtr<USkeletalMeshComponent> skeletonMesh = FindComponentByClass<USkeletalMeshComponent>();
	if (skeletonMesh)
		MainAnimInstance = skeletonMesh->GetAnimInstance();

	CurrCharacterMovementState = ECharacterMovementState::IDLE;
	CurrCharacterGait = ECharacterGait::WALK;
	CurrRotationDirection = ERotateDirection::NONE;
	NextRotationDirection = ERotateDirection::NONE;
	TargetCharacterMovementSpeed = 0;
	CurrCharacterMovementSpeed = 0;

	HasCameraInput = false;
	CameraInput = FVector2D::Zero();
	CurrViewVerticalAngle = 0;

	CurrViewHorizontalAngle = GetActorRotation().Yaw;
	CurrCharacterHorizontalAngle = CurrViewHorizontalAngle;
	CurrTargetCharacterHorizontalAngle = CurrViewHorizontalAngle;
	NextTargetCharacterHorizontalAngle = CurrTargetCharacterHorizontalAngle;
	RotationCurveScaleValue = 1;
	RotationCountdownTimer = 0;

	CurrPlayingTurnSequence = nullptr;

	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	controller->OnCharacterMovementInputStarted.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementStarted);
	controller->OnCharacterMovementInputTriggered.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementTriggered);
	controller->OnCharacterMovementInputComplete.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementComplete);
	
	controller->OnCameraMovementInputStarted.AddUObject(this, &ABase_MyCharacter::OnCameraMovementStarted);
	controller->OnCameraMovementInputTriggered.AddUObject(this, &ABase_MyCharacter::OnCameraMovementTriggered);
	controller->OnCameraMovementInputComplete.AddUObject(this, &ABase_MyCharacter::OnCameraMovementComplete);
	
	controller->OnGaitChangeInputStarted.AddUObject(this, &ABase_MyCharacter::OnGaitChangeTriggered);

	RotateCamera();
}

void ABase_MyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	if (controller)
	{
		controller->OnCharacterMovementInputStarted.RemoveAll(this);
		controller->OnCharacterMovementInputTriggered.RemoveAll(this);
		controller->OnCharacterMovementInputComplete.RemoveAll(this);
		
		controller->OnCameraMovementInputStarted.RemoveAll(this);
		controller->OnCameraMovementInputTriggered.RemoveAll(this);
		controller->OnCameraMovementInputComplete.RemoveAll(this);
		
		controller->OnGaitChangeInputStarted.RemoveAll(this);
	}
}
#pragma endregion

#pragma region Movement
void ABase_MyCharacter::OnCharacterMovementStarted()
{
	CurrCharacterMovementState = ECharacterMovementState::MOVING;
	// reset gait to walk
	CurrCharacterGait = ECharacterGait::WALK;
	SetTargetCharacterMovementSpeed();
	StopCurrentlyPlayingTurningMontage();
}

void ABase_MyCharacter::OnCharacterMovementTriggered(FVector2D movementVector)
{
	MovementInput = movementVector;
	SetTargetCharacterRotation();
	RotationCountdownTimer = 0;
}

void ABase_MyCharacter::Move(float deltaTime)
{
	FRotator characterRotation{ 0, CurrViewHorizontalAngle, 0 };
	
	FVector forwardDirection = UKismetMathLibrary::GetForwardVector(characterRotation);
	float forwardMovementAmount = MovementInput.Y * CurrCharacterMovementSpeed;
	AddMovementInput(forwardDirection, forwardMovementAmount);

	FVector rightDirection = UKismetMathLibrary::GetRightVector(characterRotation);
	float rightMovementAmount = MovementInput.X * CurrCharacterMovementSpeed;
	AddMovementInput(rightDirection, rightMovementAmount);
}

void ABase_MyCharacter::OnCharacterMovementComplete()
{
	CurrCharacterMovementState = ECharacterMovementState::IDLE;
	SetTargetCharacterMovementSpeed();
	SetTargetCharacterRotation();
	StopCurrentlyPlayingTurningMontage();
}

void ABase_MyCharacter::SetCharacterMovementSpeed(float deltaTime)
{
	if (TargetCharacterMovementSpeed > CurrCharacterMovementSpeed)
	{
		float uncappedMovementSpeed = CurrCharacterMovementSpeed + MovementSettings.CharacterAcceleration * deltaTime;
		CurrCharacterMovementSpeed = FMath::Min(uncappedMovementSpeed, TargetCharacterMovementSpeed);
	}
	else if (TargetCharacterMovementSpeed < CurrCharacterMovementSpeed)
	{
		float uncappedMovementSpeed = CurrCharacterMovementSpeed - MovementSettings.CharacterDecceleration * deltaTime;
		CurrCharacterMovementSpeed = FMath::Max(uncappedMovementSpeed, TargetCharacterMovementSpeed);
	}
}

void ABase_MyCharacter::SetTargetCharacterMovementSpeed()
{
	if (CurrCharacterMovementState == ECharacterMovementState::IDLE)
		TargetCharacterMovementSpeed = 0;
	else
		TargetCharacterMovementSpeed = CurrCharacterGait == ECharacterGait::RUN ? MovementSettings.CharacterRunMovementSpeed : MovementSettings.CharacterWalkMovementSpeed;
}

float ABase_MyCharacter::GetMovementRotation() const
{
	float dotProduct = FVector2D{ 0, 1 }.Dot(MovementInput);
	float movementRotation = FMath::Acos(dotProduct) * (180 / PI);
	if (MovementInput.X < 0)
		movementRotation = -movementRotation;
	return movementRotation;
}
#pragma endregion

#pragma region Gait
void ABase_MyCharacter::OnGaitChangeTriggered()
{
	CurrCharacterGait = CurrCharacterGait == ECharacterGait::RUN ? ECharacterGait::WALK : ECharacterGait::RUN;
	SetTargetCharacterMovementSpeed();
}
#pragma endregion

#pragma region Rotation
void ABase_MyCharacter::SetTargetCharacterRotation()
{
	if (CurrCharacterMovementState == ECharacterMovementState::MOVING)
	{
		float rotationFromMovementInput = GetMovementRotation();
		NextTargetCharacterHorizontalAngle = CurrViewHorizontalAngle + rotationFromMovementInput;
	}
	else
	{
		NextTargetCharacterHorizontalAngle = CurrViewHorizontalAngle;
	}
	
	// set target character horizontal angle to be from 0 - 360
	while (NextTargetCharacterHorizontalAngle < 0)
	{
		NextTargetCharacterHorizontalAngle += 360;
	}
	while (NextTargetCharacterHorizontalAngle > 360)
	{
		NextTargetCharacterHorizontalAngle -= 360;
	}
	
	// reverse the direction if the angle > 180
	if (FMath::Abs(CurrCharacterHorizontalAngle - NextTargetCharacterHorizontalAngle) > 180)
	{
		NextTargetCharacterHorizontalAngle = -(360 - NextTargetCharacterHorizontalAngle);
	}

	if (CurrCharacterHorizontalAngle > NextTargetCharacterHorizontalAngle)
	{
		NextRotationDirection = ERotateDirection::LEFT;
	}
	else if (CurrCharacterHorizontalAngle < NextTargetCharacterHorizontalAngle)
	{
		NextRotationDirection = ERotateDirection::RIGHT;
	}
	else
	{
		NextRotationDirection = ERotateDirection::NONE;
	}
}

void ABase_MyCharacter::UpdateCharacterMovingRotation(float deltaTime)
{
	// lerp the rotation of the character towards the target horizontal angle
	// TODO: Handle small differences and large differences
	if (CurrCharacterHorizontalAngle > CurrTargetCharacterHorizontalAngle)
	{
		float uncappedHorizontalAngle = CurrCharacterHorizontalAngle - MovementSettings.CharacterMovingRotationalSpeed * deltaTime;
		CurrCharacterHorizontalAngle = FMath::Max(uncappedHorizontalAngle, CurrTargetCharacterHorizontalAngle);
	}
	else if (CurrCharacterHorizontalAngle < CurrTargetCharacterHorizontalAngle)
	{
		float uncappedHorizontalAngle = CurrCharacterHorizontalAngle + MovementSettings.CharacterMovingRotationalSpeed * deltaTime;
		CurrCharacterHorizontalAngle = FMath::Min(uncappedHorizontalAngle, CurrTargetCharacterHorizontalAngle);
	}
}

float ABase_MyCharacter::NinetyDegreeRotationCurveAmount = 40;
float ABase_MyCharacter::OneHundredEightyDegreeRotationCurveAmount = 80;

bool ABase_MyCharacter::ShouldDoMontageRotation() const
{
	return FMath::Abs(CurrTargetCharacterHorizontalAngle - CurrCharacterHorizontalAngle) >= MovementSettings.RotationAngleThreshold;
}

bool ABase_MyCharacter::ShouldRotateInPlace() const
{
	return !IsPlayingTurningMontage() && CurrCharacterMovementState == ECharacterMovementState::IDLE && CurrCharacterMovementSpeed == 0 && ShouldDoMontageRotation();
}

bool ABase_MyCharacter::ShouldDoMovingRotation() const
{
	return !IsPlayingTurningMontage() && CurrCharacterMovementState == ECharacterMovementState::MOVING && CurrTargetCharacterHorizontalAngle != CurrCharacterHorizontalAngle;
}

void ABase_MyCharacter::SetTurnAnimationAsset()
{
	bool shouldDoBigRotation = FMath::Abs(CurrTargetCharacterHorizontalAngle - CurrCharacterHorizontalAngle) > 90;
	if (CurrRotationDirection == ERotateDirection::LEFT && shouldDoBigRotation)
	{
		CurrPlayingTurnSequence = TurnLeftMoreThan180Asset;
	}
	else if (CurrRotationDirection == ERotateDirection::LEFT && !shouldDoBigRotation)
	{
		CurrPlayingTurnSequence = TurnLeftLessThan180Asset;
	}
	else if (CurrRotationDirection == ERotateDirection::RIGHT && shouldDoBigRotation)
	{
		CurrPlayingTurnSequence = TurnRightMoreThan180Asset;
	}
	else if (CurrRotationDirection == ERotateDirection::RIGHT && !shouldDoBigRotation)
	{
		CurrPlayingTurnSequence = TurnRightLessThan180Asset;
	}
	else
	{
		CurrPlayingTurnSequence = nullptr;
	}
}

void ABase_MyCharacter::SetRotationCurveScaleValue()
{
	float diff = FMath::Abs(CurrCharacterHorizontalAngle - CurrTargetCharacterHorizontalAngle);
	if (diff <= 90)
		RotationCurveScaleValue = diff / ABase_MyCharacter::NinetyDegreeRotationCurveAmount;
	else
		RotationCurveScaleValue = diff / ABase_MyCharacter::OneHundredEightyDegreeRotationCurveAmount;
}

void ABase_MyCharacter::PlayTurningMontage()
{
	SetTurnAnimationAsset();
	SetRotationCurveScaleValue();
	float playRate = 1;
	// scale play rate if moving to match movement speed
	if (CurrCharacterMovementState == ECharacterMovementState::MOVING)
	{
		playRate = FMath::Max(MovementSettings.CharacterWalkMovementSpeed / 2, CurrCharacterMovementSpeed) / MovementSettings.CharacterWalkMovementSpeed;
	}
	MainAnimInstance->PlaySlotAnimationAsDynamicMontage(CurrPlayingTurnSequence, LegsSlotName, 0, 0, playRate);
}

void ABase_MyCharacter::StopCurrentlyPlayingTurningMontage()
{
	MainAnimInstance->StopSlotAnimation(0.25, LegsSlotName);
}

void ABase_MyCharacter::UpdateCharacterRotationThroughCurve()
{
	if (MainAnimInstance)
	{
		float rotationCurveValue = MainAnimInstance->GetCurveValue(RotationCurveName) * RotationCurveScaleValue;		
		float uncappedHorizontalAngle = CurrCharacterHorizontalAngle + rotationCurveValue;			
		if (CurrRotationDirection == ERotateDirection::LEFT)
		{
			CurrCharacterHorizontalAngle = FMath::Max(uncappedHorizontalAngle, CurrTargetCharacterHorizontalAngle);
		}
		else if (CurrRotationDirection == ERotateDirection::RIGHT)
		{
			CurrCharacterHorizontalAngle = FMath::Min(uncappedHorizontalAngle, CurrTargetCharacterHorizontalAngle);
		}
	}
}

bool ABase_MyCharacter::IsPlayingTurningMontage() const
{
	return CurrPlayingTurnSequence && MainAnimInstance->IsPlayingSlotAnimation(CurrPlayingTurnSequence, LegsSlotName);
}
#pragma endregion

#pragma region Camera
void ABase_MyCharacter::OnCameraMovementStarted()
{
	HasCameraInput = true;
}

void ABase_MyCharacter::OnCameraMovementTriggered(FVector2D cameraVector)
{
	CameraInput = cameraVector;
}

void ABase_MyCharacter::OnCameraMovementComplete()
{
	HasCameraInput = false;
}

void ABase_MyCharacter::SetTargetCameraRotation(float deltaTime)
{
	CurrViewVerticalAngle = FMath::ClampAngle(CurrViewVerticalAngle + CameraInput.Y * CameraSettings.CameraRotationalSpeed * deltaTime, CameraSettings.MinViewVerticalAngle, CameraSettings.MaxViewVerticalAngle);
	CurrViewHorizontalAngle = FMath::ClampAngle(CurrViewHorizontalAngle + CameraInput.X * CameraSettings.CameraRotationalSpeed * deltaTime, 0, 359.9);
}
#pragma endregion

#pragma region Movement and Rotation Info
FCharacterMovementSettings ABase_MyCharacter::GetMovementSettings() const
{
	return MovementSettings;
}

FCharacterState ABase_MyCharacter::GetCurrentState() const
{
	return FCharacterState{ CurrCharacterMovementState, CurrRotationDirection, CurrCharacterGait,
		CurrTargetCharacterHorizontalAngle, CurrCharacterHorizontalAngle, CurrCharacterMovementSpeed, TargetCharacterMovementSpeed };
}
#pragma endregion

void ABase_MyCharacter::Tick(float deltaTime)
{
	if (HasCameraInput)
	{
		SetTargetCameraRotation(deltaTime);
		SetTargetCharacterRotation();
	}

	if (IsPlayingTurningMontage())
	{
		UpdateCharacterRotationThroughCurve();
		SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });
	}
	else
	{
		CurrTargetCharacterHorizontalAngle = NextTargetCharacterHorizontalAngle;
		CurrRotationDirection = NextRotationDirection;
		if (ShouldDoMovingRotation())
		{
			if (ShouldDoMontageRotation())
			{
				PlayTurningMontage();
			}
			else
			{
				UpdateCharacterMovingRotation(deltaTime);
				SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });
			}
		}
		else if (ShouldRotateInPlace())
		{
			RotationCountdownTimer += deltaTime;
			if (RotationCountdownTimer >= MovementSettings.RotationLookTimeThreshold)
			{
				RotationCountdownTimer = 0;
				PlayTurningMontage();
			}
		}
		else
		{
			RotationCountdownTimer = 0;
		}
	}
	
	SetCharacterMovementSpeed(deltaTime);
	Move(deltaTime);

	RotateCamera();
}
