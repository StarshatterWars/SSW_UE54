/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         ZoneForce.cpp
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Zone Force claSS
*/


#include "ZoneForce.h"


ZoneForce::ZoneForce()
{
}

ZoneForce::~ZoneForce()
{
}

// +--------------------------------------------------------------------+

ZoneForce::ZoneForce(int i)
{
	iff = i;

	for (int n = 0; n < 8; n++)
		need[n] = 0;
}

void
ZoneForce::AddGroup(CombatGroup* group)
{
	if (group)
		groups.append(group);
}

void
ZoneForce::RemoveGroup(CombatGroup* group)
{
	if (group)
		groups.remove(group);
}

bool
ZoneForce::HasGroup(CombatGroup* group)
{
	//if (group)
	//return groups.contains(group);

	return false;
}

int
ZoneForce::GetNeed(int group_type) const
{
	switch (group_type) {
	case ECOMBATGROUP_TYPE::CARRIER_GROUP:       return need[0];
	case ECOMBATGROUP_TYPE::BATTLE_GROUP:        return need[1];
	case ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:  return need[2];
	case ECOMBATGROUP_TYPE::ATTACK_SQUADRON:     return need[3];
	case ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:    return need[4];
	case ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:  return need[5];
	}

	return 0;
}

void
ZoneForce::SetNeed(int group_type, int needed)
{
	switch (group_type) {
	case ECOMBATGROUP_TYPE::CARRIER_GROUP:       need[0] = needed; break;
	case ECOMBATGROUP_TYPE::BATTLE_GROUP:        need[1] = needed; break;
	case ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:  need[2] = needed; break;
	case ECOMBATGROUP_TYPE::ATTACK_SQUADRON:     need[3] = needed; break;
	case ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:    need[4] = needed; break;
	case ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:  need[5] = needed; break;
	}
}

void
ZoneForce::AddNeed(int group_type, int needed)
{
	switch (group_type) {
	case ECOMBATGROUP_TYPE::CARRIER_GROUP:       need[0] += needed; break;
	case ECOMBATGROUP_TYPE::BATTLE_GROUP:        need[1] += needed; break;
	case ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:  need[2] += needed; break;
	case ECOMBATGROUP_TYPE::ATTACK_SQUADRON:     need[3] += needed; break;
	case ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:    need[4] += needed; break;
	case ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:  need[5] += needed; break;
	}
}
