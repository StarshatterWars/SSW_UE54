// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "Campaign.h"


// Sets default values
ACampaign::ACampaign()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACampaign::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACampaign::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

