/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         Galaxy.h
	AUTHOR:       Carlos Bott
*/


#pragma once

#include "CoreMinimal.h"
#include "Universe.h"
#include "Galaxy.generated.h"

/**
 * 
 */

// +--------------------------------------------------------------------+

class Star;
class AStarSystem;
class Graphic;
class Light;
class Scene;

// +--------------------------------------------------------------------+

UCLASS()
class STARSHATTERWARS_API AGalaxy : public AUniverse
{
	GENERATED_BODY()

public:	
	AGalaxy();
	AGalaxy(const char* name);
	~AGalaxy();

	int operator == (const AGalaxy& s)   const { return name == s.name; }

	// operations:
	virtual void         Load();
	virtual void         Load(const char* filename);

	void				 SpawnSystem(FString name);

	void				 SpawnSystem(FString sysName, FVector sysLoc, int sysIFF, int starClass);

	virtual void         ExecFrame();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// accessors:
	const char* Name()         const { return name; }
	const char* Description()  const { return description; }
	List<AStarSystem>& GetSystemList() { return systems; }
	List<Star>& Stars() { return stars; }
	double               Radius()       const { return radius; }

	AStarSystem* GetSystem(const char* name);
	AStarSystem* FindSystemByRegion(const char* rgn_name);

	static void         Close();
	static AGalaxy*		GetInstance();

protected:
	char                 filename[64];
	Text                 name;
	Text                 description;
	double               radius;           // radius in parsecs

	List<AStarSystem>    systems;
	List<Star>           stars;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	USceneComponent* Root;
};

