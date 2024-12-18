/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         MissionTemplate.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Mission Template classes
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/Text.h"
#include "Mission.h"


// +--------------------------------------------------------------------+

class MissionTemplate;
class MissionElement;
class MissionEvent;
class Element;
class ComatGroup;
class MissionAlias;
class MissionCallsign;
class MissionEvent;

// +--------------------------------------------------------------------+

/**
 * 
 */
class STARSHATTERWARS_API MissionTemplate : public Mission
{
public:
	static const char* TYPENAME() { return "MissionTemplate"; }

	MissionTemplate(int id, const char* filename = 0, const char* path = 0);
	virtual ~MissionTemplate();

	virtual bool         Load(const char* filename = 0, const char* path = 0);

	// accessors/mutators:
	virtual MissionElement* FindElement(const char* name);
	virtual void         AddElement(MissionElement* elem);
	virtual bool         MapElement(MissionElement* elem);
	virtual Text         MapShip(Text name);
	virtual CombatGroup* GetPlayerSquadron()           const { return player_squadron; }
	virtual void         SetPlayerSquadron(CombatGroup* ps) { player_squadron = ps; }
	virtual Text         MapCallsign(const char* name, int iff);
	virtual bool         MapEvent(MissionEvent* event);


protected:
	CombatGroup* FindCombatGroup(int iff, const ShipDesign* dsn);
	void                 ParseAlias(TermStruct* val);
	void                 ParseCallsign(TermStruct* val);
	bool                 ParseOptional(TermStruct* val);
	void                 CheckObjectives();

	List<MissionAlias>      aliases;
	List<MissionCallsign>   callsigns;
	CombatGroup* player_squadron;
};

// +--------------------------------------------------------------------+

class MissionAlias
{
	friend class MissionTemplate;

public:
	static const char* TYPENAME() { return "MissionAlias"; }

	MissionAlias() : elem(0) { }
	MissionAlias(const char* n, MissionElement* e) : name(n), elem(e) { }
	virtual ~MissionAlias() { }

	int operator == (const MissionAlias& a)   const { return name == a.name; }

	Text              Name()                  const { return name; }
	MissionElement* Element()               const { return elem; }

	void              SetName(const char* n) { name = n; }
	void              SetElement(MissionElement* e) { elem = e; }

protected:
	Text              name;
	MissionElement* elem;
};

// +--------------------------------------------------------------------+

class MissionCallsign
{
	friend class MissionTemplate;

public:
	static const char* TYPENAME() { return "MissionCallsign"; }

	MissionCallsign() { }
	MissionCallsign(const char* c, const char* n) : call(c), name(n) { }
	virtual ~MissionCallsign() { }

	int operator == (const MissionCallsign& a)const { return call == a.call; }

	Text              Callsign()              const { return call; }
	Text              Name()                  const { return name; }

	void              SetCallsign(const char* c) { call = c; }
	void              SetName(const char* n) { name = n; }

protected:
	Text              call;
	Text              name;
};


