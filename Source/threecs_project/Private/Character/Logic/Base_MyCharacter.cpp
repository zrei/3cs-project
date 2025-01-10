// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Logic/Base_MyCharacter.h"
#include "Controller/MyPlayerController.h"
#include "Math/UnrealMathUtility.h"

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
	MovementSettings.StationaryRotationThreshold = 5;

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

	TObjectPtr<USkeletalMeshComponent> skeletalMesh = FindComponentByClass<USkeletalMeshComponent>();
	if (skeletalMesh)
		MainAnimInstance = skeletalMesh->GetAnimInstance();

	CurrCharacterMovementState = ECharacterMovementState::IDLE;
	CurrCharacterGait = ECharacterGait::WALK;
	CurrRotationDirection = ERotateDirection::NONE;
	TargetCharacterMovementSpeed = 0;
	CurrCharacterMovementSpeed = 0;

	HasCameraInput = false;
	CameraInput = FVector2D::Zero();
	CurrViewVerticalAngle = 0;

	CurrViewHorizontalAngle = GetActorRotation().Yaw;
	CurrCharacterHorizontalAngle = CurrViewHorizontalAngle;
	TargetCharacterHorizontalAngle = CurrViewHorizontalAngle;

	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	controller->OnCharacterMovementInputStarted.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementStarted);
	controller->OnCharacterMovementInputTriggered.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementTriggered);
	controller->OnCharacterMovementInputComplete.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementComplete);
	
	controller->OnCameraMovementInputStarted.AddUObject(this, &ABase_MyCharacter::OnCameraMovementStarted);
	controller->OnCameraMovementInputTriggered.AddUObject(this, &ABase_MyCharacter::OnCameraMovementTriggered);
	controller->OnCameraMovementInputComplete.AddUObject(this, &ABase_MyCharacter::OnCameraMovementComplete);
	
	controller->OnGaitChangeInputStarted.AddUObject(this, &ABase_MyCharacter::OnGaitChangeTriggered);

	RotationCurveScaleValue = 1;

	CurrPlayingMontage = nullptr;

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
}

void ABase_MyCharacter::OnCharacterMovementTriggered(FVector2D movementVector)
{
	MovementInput = movementVector;
	SetTargetCharacterRotation();
}

void ABase_MyCharacter::Move(float deltaTime)
{
	FVector forwardDirection = GetActorForwardVector();
	float forwardMovementAmount = MovementInput.Y * CurrCharacterMovementSpeed;
	AddMovementInput(forwardDirection, forwardMovementAmount);

	FVector rightDirection = GetActorRightVector();
	float rightMovementAmount = MovementInput.X * CurrCharacterMovementSpeed;
	AddMovementInput(rightDirection, rightMovementAmount);
}

void ABase_MyCharacter::OnCharacterMovementComplete()
{
	CurrCharacterMovementState = ECharacterMovementState::IDLE;
	SetTargetCharacterMovementSpeed();
	SetTargetCharacterRotation();
	MainAnimInstance->StopSlotAnimation(0.25, LegsSlotName);
	RotationCurveScaleValue = 1;
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
	// set target character horizontal angle to be from 0 - 360
	if (CurrCharacterMovementState == ECharacterMovementState::MOVING)
	{
		float rotationFromMovementInput = GetMovementRotation();
		TargetCharacterHorizontalAngle = CurrViewHorizontalAngle + rotationFromMovementInput;
	}
	else
	{
		TargetCharacterHorizontalAngle = CurrViewHorizontalAngle;
	}
	
	TargetCharacterHorizontalAngle = TargetCharacterHorizontalAngle < 0 ? 360 + TargetCharacterHorizontalAngle : TargetCharacterHorizontalAngle;
	// reverse the direction if the angle > 180
	if (FMath::Abs(CurrCharacterHorizontalAngle - TargetCharacterHorizontalAngle) > 180)
	{
		TargetCharacterHorizontalAngle = -(360 - TargetCharacterHorizontalAngle);
	}

	if (CurrCharacterHorizontalAngle > TargetCharacterHorizontalAngle)
	{
		CurrRotationDirection = ERotateDirection::LEFT;
	}
	else if (CurrCharacterHorizontalAngle < TargetCharacterHorizontalAngle)
	{
		CurrRotationDirection = ERotateDirection::RIGHT;
	}
	else
	{
		CurrRotationDirection = ERotateDirection::NONE;
	}
}

void ABase_MyCharacter::UpdateCharacterMovingRotation(float deltaTime)
{
	// lerp the rotation of the character towards the target horizontal angle
	// TODO: Handle small differences and large differences
	if (CurrCharacterHorizontalAngle > TargetCharacterHorizontalAngle)
	{
		float uncappedHorizontalAngle = CurrCharacterHorizontalAngle - MovementSettings.CharacterMovingRotationalSpeed * deltaTime;
		CurrCharacterHorizontalAngle = FMath::Max(uncappedHorizontalAngle, TargetCharacterHorizontalAngle);
		
	}
	else if (CurrCharacterHorizontalAngle < TargetCharacterHorizontalAngle)
	{
		float uncappedHorizontalAngle = CurrCharacterHorizontalAngle + MovementSettings.CharacterMovingRotationalSpeed * deltaTime;
		CurrCharacterHorizontalAngle = FMath::Min(uncappedHorizontalAngle, TargetCharacterHorizontalAngle);
	}
}

bool ABase_MyCharacter::ShouldRotateInPlace() const
{
	return CurrCharacterMovementState == ECharacterMovementState::IDLE && FMath::Abs(CurrCharacterHorizontalAngle - TargetCharacterHorizontalAngle) > MovementSettings.StationaryRotationThreshold;
}

bool ABase_MyCharacter::ShouldDoMovingRotation() const
{
	return CurrCharacterMovementState == ECharacterMovementState::MOVING && TargetCharacterHorizontalAngle != CurrCharacterHorizontalAngle && (!CurrPlayingMontage || !MainAnimInstance->IsPlayingSlotAnimation(CurrPlayingMontage, LegsSlotName));
}

bool ABase_MyCharacter::ShouldDoMontageRotation() const
{
	return FMath::Abs(TargetCharacterHorizontalAngle - CurrCharacterHorizontalAngle) >= 45;
}

TObjectPtr<UAnimSequenceBase> ABase_MyCharacter::GetTurnAnimationAsset()
{
	bool shouldDoBigRotation = FMath::Abs(TargetCharacterHorizontalAngle - CurrCharacterHorizontalAngle) > 90;
	if (CurrRotationDirection == ERotateDirection::LEFT && shouldDoBigRotation)
	{
		return TurnLeftMoreThan180Asset;
	}
	else if (CurrRotationDirection == ERotateDirection::LEFT && !shouldDoBigRotation)
	{
		return TurnLeftLessThan180Asset;
	}
	else if (CurrRotationDirection == ERotateDirection::RIGHT && shouldDoBigRotation)
	{
		return TurnRightMoreThan180Asset;
	}
	else if (CurrRotationDirection == ERotateDirection::RIGHT && !shouldDoBigRotation)
	{
		return TurnRightLessThan180Asset;
	}
	else
	{
		return nullptr;
	}
}

void ABase_MyCharacter::UpdateCharacterRotationThroughCurve(float deltaTime)
{
	if (MainAnimInstance)
	{
		float rotationCurveValue = MainAnimInstance->GetCurveValue(RotationCurveName);
		float uncappedHorizontalAngle = CurrCharacterHorizontalAngle + rotationCurveValue * MovementSettings.CharacterStationaryRotationalSpeed * deltaTime;
		if (CurrCharacterHorizontalAngle > TargetCharacterHorizontalAngle)
		{
			CurrCharacterHorizontalAngle = FMath::Max(uncappedHorizontalAngle, TargetCharacterHorizontalAngle);
		}
		else if (CurrCharacterHorizontalAngle < TargetCharacterHorizontalAngle)
		{
			CurrCharacterHorizontalAngle = FMath::Min(uncappedHorizontalAngle, TargetCharacterHorizontalAngle);
		}
	}
}

void ABase_MyCharacter::UpdateCharacterRotationThroughCurveWhenMoving()
{
	if (MainAnimInstance)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Scale value: %f"), RotationCurveScaleValue);
		float rotationCurveValue = MainAnimInstance->GetCurveValue(RotationCurveName) * RotationCurveScaleValue;
		if (!CurrPlayingMontage && MainAnimInstance->IsPlayingSlotAnimation(CurrPlayingMontage, LegsSlotName))
			UE_LOG(LogTemp, Warning, TEXT("Rotation curve value: %f"), rotationCurveValue);
		float uncappedHorizontalAngle = CurrCharacterHorizontalAngle + rotationCurveValue;
		if (CurrCharacterHorizontalAngle > TargetCharacterHorizontalAngle)
		{
			CurrCharacterHorizontalAngle = FMath::Max(uncappedHorizontalAngle, TargetCharacterHorizontalAngle);
		}
		else if (CurrCharacterHorizontalAngle < TargetCharacterHorizontalAngle)
		{
			CurrCharacterHorizontalAngle = FMath::Min(uncappedHorizontalAngle, TargetCharacterHorizontalAngle);
		}
	}
}

float ABase_MyCharacter::NinetyDegreeRotationCurveAmount = 40;
float ABase_MyCharacter::OneHundredEightyDegreeRotationCurveAmount = 80;
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
		TargetCharacterHorizontalAngle, CurrCharacterHorizontalAngle, CurrCharacterMovementSpeed, TargetCharacterMovementSpeed };
}
#pragma endregion

void ABase_MyCharacter::Tick(float deltaTime)
{
	if (HasCameraInput)
	{
		SetTargetCameraRotation(deltaTime);
		SetTargetCharacterRotation();
	}

	if (ShouldDoMovingRotation())
	{
		UE_LOG(LogTemp, Warning, TEXT("Should do moving rotation"));
		if (ShouldDoMontageRotation())
		{
			CurrPlayingMontage = GetTurnAnimationAsset();
			float diff = FMath::Abs(CurrCharacterHorizontalAngle - TargetCharacterHorizontalAngle);
			if (diff < 180)
				RotationCurveScaleValue = diff / ABase_MyCharacter::NinetyDegreeRotationCurveAmount;
			else
				RotationCurveScaleValue = diff / ABase_MyCharacter::OneHundredEightyDegreeRotationCurveAmount;
			UE_LOG(LogScript, Warning, TEXT("Angle diff: %f"), diff);
			UE_LOG(LogScript, Warning, TEXT("Rotation curva scale value: %f"), RotationCurveScaleValue);
			MainAnimInstance->PlaySlotAnimationAsDynamicMontage(CurrPlayingMontage, LegsSlotName);
		}
		else
		{
			UpdateCharacterMovingRotation(deltaTime);
			SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });
		}
	}
	else if (ShouldRotateInPlace())
	{
		UpdateCharacterRotationThroughCurve(deltaTime);
		SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });
	}
	else
	{
		UpdateCharacterRotationThroughCurveWhenMoving();
		SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });
	}
	

	SetCharacterMovementSpeed(deltaTime);
	Move(deltaTime);

	RotateCamera();
}
