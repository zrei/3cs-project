// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

struct FInputActionInstance;
class UCameraComponent;
class ABase_MyCharacter;

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
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

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
#pragma endregion

#pragma region Rope
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> RopeTop;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> Rope;

protected:
	UPROPERTY(EditAnywhere, Category="Rope Settings")
	FName SimulatePhysicsBone;

#pragma endregion

#pragma region Camera
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCameraComponent> RopeCamera;

protected:
	UPROPERTY(EditAnywhere)
	float CameraOffset;

private:
	void UpdateCameraPositionAndRotation();

	float DefaultLookPitch;
#pragma endregion

#pragma region Collision
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> SphereCollision;

private:
	UFUNCTION()
	void OnCharacterOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	inline void ActivateCollision()
	{
		SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ARope::OnCharacterOverlap);
	}

	inline void DeactivateCollision()
	{
		// TODO: Test if this crashes if you are attached and you end the game
		SphereCollision->OnComponentBeginOverlap.RemoveAll(this);
	}

	static constexpr float SphereCollisionRadius = 5;
#pragma endregion

#pragma region Attach
private:
	bool TryAttachCharacter(ABase_MyCharacter* character);

	TObjectPtr<ABase_MyCharacter> AttachedCharacter;

	float CharacterAttachHorizontalAngle;

public:
	static FRopeAttachDelegate RopeAttachEvent;

protected:
	UPROPERTY(EditAnywhere)
	FName AttachBone;
#pragma endregion

#pragma region Detach
private:
	bool TryDetachCharacter();

public:
	static FRopeDetachDelegate RopeDetachEvent;

protected:
	UPROPERTY(EditAnywhere)
	float SwingCooldown;
#pragma endregion

#pragma region Input
private:
	void OnMovementTriggered(const FInputActionInstance& input) const;

	void SubscribeToMovement();

	void UnsubcribeToMovement();

	void StartJump(const FInputActionInstance& input);

	void OnJumpTriggered(const FInputActionInstance& input);

	bool HasJumpInputStarted;

protected:
	UPROPERTY(EditAnywhere)
	float RopeForce;

	UPROPERTY(EditAnywhere)
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
