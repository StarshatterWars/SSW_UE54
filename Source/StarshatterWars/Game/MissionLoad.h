/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         MissionShip.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Simulation Universe and Region classes
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
//#include "Intel.h"
#include "../Foundation/RLoc.h"
#include "../Space/Universe.h"
//#include "Scene.h"
//#include "Skin.h"
//#include "Physical.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/List.h"
#include "../Foundation/Text.h"
#include "../System/SSWGameInstance.h"
/**
 * 
 */
class STARSHATTERWARS_API MissionLoad
{
public:
	
	friend class Mission;

	static const char* TYPENAME() { return "MissionLoad"; }
	
	MissionLoad();
	~MissionLoad();

	void Initialize(int Ship = -1, const char* Name = 0);

	int               GetShip() const;
	void              SetShip(int ship);

	Text              GetName() const;
	void              SetName(Text name);

	int* GetStations();
	int               GetStation(int index);
	void              SetStation(int index, int selection);

protected:
	int               ship;
	Text              name;
	int               load[16];
};
