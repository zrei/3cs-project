// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

struct FInputActionInstance;
class UCameraComponent;
class ABase_MyCharacter;
class URopeSettings;

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Data/Rope/RopeSettings.h"
#include "Rope.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FRopeAttachDelegate, ARope* const);
DECLARE_MULTICAST_DELEGATE(FRopeDetachDelegate);

UCLASS()
class THREECS_PROJECT_API ARope : public AActor
{
	GENERATED_BODY()

#pragma region Initialisation
public:	
	// Sets default values for this actor's properties
	ARope();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere)
	TObjectPtr<URopeSettings> RopeSettings;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	static constexpr unsigned int TOTAL_BONES = 120;

private:
	inline FString GetBoneName(unsigned int boneNumber)
	{
		FString boneNumberString = FString::FromInt(boneNumber);
		// this is so hacky
		if (boneNumber < 10)
		{
			boneNumberString = FString{"00"} + boneNumberString;
		}
		else if (boneNumber < 100)
		{
			boneNumberString = FString{ "0" } + boneNumberString;
		}
		return RopeSettings->BonePrefix + boneNumberString;
	}

	FName BoneToSimulatePhysics;

#pragma endregion

#pragma region Rope
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> RopeTop;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> Rope;

#pragma endregion

#pragma region Camera
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCameraComponent> RopeCamera;

private:
	float CameraOffset;

private:
	void UpdateCameraPositionAndRotation();

	float DefaultLookPitch;
#pragma endregion

#pragma region Collision
private:
	UFUNCTION()
	void OnCharacterOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	inline void ActivateCollision()
	{
		for (const TObjectPtr<USceneComponent>& child : Rope->GetAttachChildren())
		{
			TObjectPtr<USphereComponent> sphere = Cast<USphereComponent>(child);
			if (!sphere)
				continue;
			sphere->OnComponentBeginOverlap.AddDynamic(this, &ARope::OnCharacterOverlap);
		}
	}

	inline void DeactivateCollision()
	{
		for (const TObjectPtr<USceneComponent>& child : Rope->GetAttachChildren())
		{
			TObjectPtr<USphereComponent> sphere = Cast<USphereComponent>(child);
			if (!sphere)
				continue;
			sphere->OnComponentBeginOverlap.RemoveAll(this);
		}
	}
#pragma endregion

#pragma region Attach
private:
	bool TryAttachCharacter(ABase_MyCharacter* character, const FName& attachBone);

	TObjectPtr<ABase_MyCharacter> AttachedCharacter;

	float CharacterAttachHorizontalAngle;

	constexpr static float InitialVelocityMultiplier = 5;

public:
	static FRopeAttachDelegate RopeAttachEvent;

#pragma endregion

#pragma region Detach
private:
	bool TryDetachCharacter();

	constexpr static float LaunchVelocityMultiplier = 5;

public:
	static FRopeDetachDelegate RopeDetachEvent;
#pragma endregion

#pragma region Input
private:
	void OnMovementTriggered(const FInputActionInstance& input) const;

	void OnMovementCompleted(const FInputActionInstance& input) const;

	void SubscribeToMovement();

	void UnsubscribeToMovement();

	void StartJump(const FInputActionInstance& input);

	void OnJumpTriggered(const FInputActionInstance& input);

	bool HasJumpInputStarted;

	FName BoneToApplyForce;
#pragma endregion

#pragma region State
private:
	bool IsOccupied;

	bool CanSwing;

	void ResetSwingableState();

	FTimerHandle ResetSwingStateTimer;
#pragma endregion
};
