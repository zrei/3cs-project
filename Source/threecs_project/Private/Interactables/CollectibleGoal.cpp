// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/CollectibleGoal.h"
#include "Character/Logic/Base_MyCharacter.h"
#include "Components/SphereComponent.h"

// Sets default values
ACollectibleGoal::ACollectibleGoal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Interaction = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction"));
	Interaction->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ACollectibleGoal::BeginPlay()
{
	Super::BeginPlay();

	Interaction->OnComponentBeginOverlap.AddDynamic(this, &ACollectibleGoal::OnOverlap);
}

void ACollectibleGoal::EndPlay(EEndPlayReason::Type endPlayReason)
{
	Super::EndPlay(endPlayReason);

	Interaction->OnComponentBeginOverlap.RemoveAll(this);
}

void ACollectibleGoal::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TObjectPtr<ABase_MyCharacter> character = Cast<ABase_MyCharacter>(OtherActor);
	if (character)
		OnCollect();
}
