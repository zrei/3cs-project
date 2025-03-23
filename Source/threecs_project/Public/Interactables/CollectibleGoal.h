// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UStaticMeshComponent;
class USphereComponent;

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CollectibleGoal.generated.h"

UCLASS()
class THREECS_PROJECT_API ACollectibleGoal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACollectibleGoal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type);

	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> Interaction;

	UFUNCTION(BlueprintImplementableEvent)
	void OnCollect();

private:
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
