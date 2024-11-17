/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         GameDataLoader.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Loader and Parser class for initial generation of Master Game Data Tables
	Will not be used after Dable Table is Generated.
*/



#include "GameDataLoader.h"
#include "../System/SSWGameInstance.h"
#include "../System/Game.h"
#include "../Space/Starsystem.h"
#include "CombatGroup.h"
#include "CombatRoster.h"
#include "CombatAction.h"
#include "CombatActionReq.h"
#include "CombatEvent.h"
#include "Combatant.h"
#include "Mission.h"
#include "Intel.h"
#include "PlayerData.h"


// Sets default values
AGameDataLoader::AGameDataLoader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::AGameDataLoader()"));

	static ConstructorHelpers::FObjectFinder<UDataTable> CampaignDataTableObject(TEXT("DataTable'/Game/Game/DT_Campaign.DT_Campaign'"));

	if (CampaignDataTableObject.Succeeded())
	{
		CampaignDataTable = CampaignDataTableObject.Object;
		CampaignDataTable->EmptyTable();
	}

}

// Called when the game starts or when spawned
void AGameDataLoader::BeginPlay()
{
	Super::BeginPlay();
	GetSSWInstance();
	InitializeCampaignData();
}

// Called every frame
void AGameDataLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGameDataLoader::LoadGalaxyData()
{

}

void AGameDataLoader::GetSSWInstance()
{
	SSWInstance = (USSWGameInstance*)GetGameInstance();
}

void AGameDataLoader::InitializeCampaignData() {
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadCampaignData"));

	FString ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Campaigns/"));
	FString FileName = ProjectPath;

	for (int i = 1; i < 6; i++) {		
		FileName = ProjectPath; 
		FileName.Append("0");
		FileName.Append(FString::FormatAsNumber(i));
		FileName.Append("/");
		CampaignPath = FileName;
		FileName.Append("campaign.def");
		LoadCampaignData(TCHAR_TO_ANSI(*FileName), true);
	}
}

void AGameDataLoader::LoadCampaignData(const char* FileName, bool full)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadCampaignData"));
	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(FileName);

	FString fs = FString(ANSI_TO_TCHAR(FileName));
	FString FileString;
	BYTE* block = 0;

	SSWInstance->loader->LoadBuffer(FileName, block, true);

	UE_LOG(LogTemp, Log, TEXT("Loading Campaign Data: %s"), *fs);

	if (FFileHelper::LoadFileToString(FileString, *fs, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);

		const char* result = TCHAR_TO_ANSI(*FileString);
	}

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	if (!term) {
		UE_LOG(LogTemp, Log, TEXT("WARNING: could not parse '%s'"), *fs);
		return;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Campaign file '%s'"), *fs);
	}

	FS_Campaign NewCampaignData;
	TArray<FString> OrdersArray;
	OrdersArray.Empty();

	CombatantArray.Empty();
	CampaignActionArray.Empty();
	MissionArray.Empty();

	do {
		delete term; 
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {

				if (def->name()->value() == "name") {
						GetDefText(name, def, filename);
						NewCampaignData.Name = FString(name);
				}
				else if (def->name()->value() == "desc") {
						GetDefText(description, def, filename);
						NewCampaignData.Description = FString(description);
				}
				else if (def->name()->value() == "situation") {
						GetDefText(situation, def, filename);
						NewCampaignData.Situation = FString(situation);
				}
				else if (def->name()->value() == "orders") {
						GetDefText(orders, def, filename);
						OrdersArray.Add(FString(orders));
						NewCampaignData.Orders = OrdersArray;
				}
				else if (def->name()->value() == "scripted") {
					if (def->term() && def->term()->isBool()) {
						scripted = def->term()->isBool()->value();
						NewCampaignData.Scripted = scripted;
					}
				}
				else if (def->name()->value() == "sequential") {
					if (def->term() && def->term()->isBool()) {
						sequential = def->term()->isBool()->value();
						NewCampaignData.Sequential = sequential;
					}
				}

				else if (def->name()->value() == "combatant_groups") {
					GetDefNumber(CombatantSize, def, filename);
					NewCampaignData.CombatantSize = CombatantSize;
				}
	
				else if (def->name()->value() == "action_groups") {
					GetDefNumber(ActionSize, def, filename);
					NewCampaignData.ActionSize = ActionSize;
				}

				else if (def->name()->value() == "action") {
					
					TermStruct* ActionTerm = def->term()->isStruct();

					NewCampaignData.ActionSize = ActionTerm->elements()->size();
		
					if (NewCampaignData.ActionSize > 0)
					{
						ActionId = 0;
						ActionType = "";
						ActionSubtype = 0;
						OppType = -1;
						ActionTeam = 0;
						ActionSource = "";
						ActionLocation = Vec3(0.0f, 0.0f, 0.0f);
						
						ActionSystem = "";
						ActionRegion = "";
						ActionFile = "";
						ActionImage = "";
						ActionScene = "";
						ActionText = "";

						ActionCount = 1;
						StartBefore = Game::TIME_NEVER;
						StartAfter = 0;
						MinRank = 0;
						MaxRank = 100;
						Delay = 0 ;
						Probability = 100;

						AssetType = "";
						AssetId = 0;
						TargetType = "";
						TargetId = 0;
						TargetIff = 0;

						AssetKill = "";
						TargetKill = "";

						for (int ActionIdx = 0; ActionIdx < NewCampaignData.ActionSize; ActionIdx++)
						{
							TermDef* pdef = ActionTerm->elements()->at(ActionIdx)->isDef();

							if (pdef->name()->value() == "id") {
								GetDefNumber(ActionId, pdef, filename);
								NewCampaignAction.Id = ActionId;
								UE_LOG(LogTemp, Log, TEXT("action id: '%d'"), ActionId);
							}
							else if (pdef->name()->value() == "type") {
								GetDefText(ActionType, pdef, filename);
								//type = CombatAction::TypeFromName(txt);
								NewCampaignAction.Type = FString(ActionType);
								UE_LOG(LogTemp, Log, TEXT("action type: '%s'"), *FString(ActionType));
							}
							else if (pdef->name()->value() == "subtype") {
								if (pdef->term()->isNumber()) {
									GetDefNumber(ActionSubtype, pdef, filename);
									NewCampaignAction.Subtype = ActionSubtype;
								}
								else if (pdef->term()->isText()) {
									char txt[64];
									GetDefText(txt, pdef, filename);

									//if (type == CombatAction::MISSION_TEMPLATE) {
									//	ActionSubtype = Mission::TypeFromName(txt);
									//}
									//else if (type == CombatAction::COMBAT_EVENT) {
									//	ActionSubtype = CombatEvent::TypeFromName(txt);
									//}
									if (ActionType == CombatAction::INTEL_EVENT) {
										ActionSubtype = Intel::IntelFromName(txt);
									}
									NewCampaignAction.Subtype = ActionSubtype;
								}

							}
							else if (pdef->name()->value() == "opp_type") {
								if (pdef->term()->isNumber()) {
									GetDefNumber(OppType, pdef, filename);
									NewCampaignAction.OppType = OppType;
								}

								/*else if (pdef->term()->isText()) {
									GetDefText(txt, pdef, filename);

									if (type == CombatAction::MISSION_TEMPLATE) {
										opp_type = Mission::TypeFromName(txt);
									}
								}*/
							}
							else if (pdef->name()->value() == "source") {
								GetDefText(ActionSource, pdef, filename);
								//source = CombatEvent::SourceFromName(txt);
								NewCampaignAction.Source = FString(ActionSource);
							}
							else if (pdef->name()->value() == "team") {
								GetDefNumber(ActionTeam, pdef, filename);
								NewCampaignAction.Team = ActionTeam;
							}
							else if (pdef->name()->value() == "iff") {
								GetDefNumber(ActionTeam, pdef, filename);
								NewCampaignAction.Iff = ActionTeam;
							}
							else if (pdef->name()->value() == "count") {
								GetDefNumber(ActionCount, pdef, filename);
								NewCampaignAction.Count = ActionCount;
							}
							else if (pdef->name()->value().contains("before")) {
								if (pdef->term()->isNumber()) {
									GetDefNumber(StartBefore, pdef, filename);
									NewCampaignAction.StartBefore = StartBefore;
								}
								else {
									GetDefTime(StartBefore, pdef, filename);
									StartBefore -= Game::ONE_DAY;
									NewCampaignAction.StartBefore = StartBefore;
								}
							}
							else if (pdef->name()->value().contains("after")) {
								if (pdef->term()->isNumber()) {
									GetDefNumber(StartAfter, pdef, filename);
									NewCampaignAction.StartAfter = StartAfter;
								}
								else {
									GetDefTime(StartAfter, pdef, filename);
									StartAfter -= Game::ONE_DAY;
									NewCampaignAction.StartAfter = StartAfter;
								}
							}
							else if (pdef->name()->value() == "min_rank") {
								if (pdef->term()->isNumber()) {
									GetDefNumber(MinRank, pdef, filename);
									NewCampaignAction.MinRank = MinRank;
								}
								else {
									char rank_name[64];
									GetDefText(rank_name, pdef, filename);
									MinRank = PlayerData::RankFromName(rank_name);
									NewCampaignAction.MinRank = MinRank;
								}
							}
							else if (pdef->name()->value() == "max_rank") {
								if (pdef->term()->isNumber()) {
									GetDefNumber(MaxRank, pdef, filename);
									NewCampaignAction.MaxRank = MaxRank;
								}
								else {
									char rank_name[64];
									GetDefText(rank_name, pdef, filename);
									MaxRank = PlayerData::RankFromName(rank_name);
									NewCampaignAction.MaxRank = MaxRank;
								}
							}
							else if (pdef->name()->value() == "delay") {
								GetDefNumber(Delay, pdef, filename);
								NewCampaignAction.Delay = Delay;
							}
							else if (pdef->name()->value() == "probability") {
								GetDefNumber(Probability, pdef, filename);
								NewCampaignAction.Probability = Probability;
							}
							else if (pdef->name()->value() == "asset_type") {
								GetDefText(AssetType, pdef, filename);
								//asset_type = CombatGroup::TypeFromName(type_name);
								NewCampaignAction.AssetType = FString(AssetType);
							}
							else if (pdef->name()->value() == "target_type") {
								GetDefText(TargetType, pdef, filename);
								NewCampaignAction.TargetType = FString(TargetType);
								//target_type = CombatGroup::TypeFromName(type_name);
							}
							else if (pdef->name()->value() == "location" ||
								pdef->name()->value() == "loc") {
								GetDefVec(ActionLocation, pdef, filename);
								NewCampaignAction.Location.X = ActionLocation.x;
								NewCampaignAction.Location.Y = ActionLocation.y;
								NewCampaignAction.Location.Z = ActionLocation.z;

							}
							else if (pdef->name()->value() == "system" ||
								pdef->name()->value() == "sys") {
								GetDefText(ActionSystem, pdef, filename);
								NewCampaignAction.System = FString(ActionSystem);
							}
							else if (pdef->name()->value() == "region" ||
								pdef->name()->value() == "rgn" ||
								pdef->name()->value() == "zone") {
								GetDefText(ActionRegion, pdef, filename);
								NewCampaignAction.Region = FString(ActionRegion);
							}
							else if (pdef->name()->value() == "file") {
								GetDefText(ActionFile, pdef, filename);
								NewCampaignAction.File = FString(ActionFile);
							}
							else if (pdef->name()->value() == "image") {
								GetDefText(ActionImage, pdef, filename);
								NewCampaignAction.Image = FString(ActionImage);
							}
							else if (pdef->name()->value() == "scene") {
								GetDefText(ActionScene, pdef, filename);
								NewCampaignAction.Scene = FString(ActionScene);
							}
							else if (pdef->name()->value() == "text") {
								GetDefText(ActionText, pdef, filename);
								NewCampaignAction.Text = FString(ActionText);
							}
							else if (pdef->name()->value() == "asset_id") {
								GetDefNumber(AssetId, pdef, filename);
								NewCampaignAction.AssetId = AssetId;
							}
							else if (pdef->name()->value() == "target_id") {
								GetDefNumber(TargetId, pdef, filename);
								NewCampaignAction.TargetId = TargetId;
							}

							else if (pdef->name()->value() == "target_iff") {
								GetDefNumber(TargetIff, pdef, filename);
								NewCampaignAction.TargetIff = TargetIff;
							}
							else if (pdef->name()->value() == "asset_kill") {
								GetDefText(AssetKill, pdef, filename);
								NewCampaignAction.AssetKill = FString(AssetKill);
							}

							else if (pdef->name()->value() == "target_kill") {
								GetDefText(TargetKill, pdef, filename);
								NewCampaignAction.TargetKill = FString(TargetKill);
							}
						}
						CampaignActionArray.Add(NewCampaignAction);
					}
					NewCampaignData.Action = CampaignActionArray;
				}

				else if (def->name()->value() == "combatant") {
					
					TermStruct* CombatantTerm = def->term()->isStruct();

					NewCampaignData.CombatantSize = CombatantTerm->elements()->size();

					if (NewCampaignData.CombatantSize > 0)
					{
						// Add Unit Stuff Here

						CombatantName = "";
						CombatantSize = 0;
						NewCombatUnit.Group.Empty();

						for (int UnitIdx = 0; UnitIdx < NewCampaignData.CombatantSize; UnitIdx++)
						{	
							def = CombatantTerm->elements()->at(UnitIdx)->isDef();

							if (def->name()->value() == "name") {
								GetDefText(CombatantName, def, filename);
								NewCombatUnit.Name = FString(CombatantName);
							} else if (def->name()->value() == "size") {
								GetDefNumber(CombatantSize, def, filename);
								NewCombatUnit.Size = CombatantSize;
							} else if (def->name()->value() == "group") {
								//ParseGroup(def->term()->isStruct(), filename);
								TermStruct* GroupTerm = def->term()->isStruct();

								CombatantType = "";
								CombatantId = 0;
								
								for (int i = 0; i < GroupTerm->elements()->size(); i++) {

									TermDef* pdef = GroupTerm->elements()->at(i)->isDef();
									if (pdef->name()->value() == "type") {
										GetDefText(CombatantType, pdef, filename);
										NewGroupUnit.Type = FString(CombatantType);
										UE_LOG(LogTemp, Log, TEXT("%s:  %s"), *FString(pdef->name()->value()), *FString(CombatantType));
										//type = CombatGroup::TypeFromName(type_name);
									}

									else if (pdef->name()->value() == "id") {
										GetDefNumber(CombatantId, pdef, filename);
										NewGroupUnit.Id = CombatantId;
										UE_LOG(LogTemp, Log, TEXT("%s: %d"), *FString(pdef->name()->value()), CombatantId);
									}
									else if (pdef->name()->value() == "req") {
										
										TermStruct* val2 = pdef->term()->isStruct();

						

										
									}
								}
								NewCombatUnit.Group.Add(NewGroupUnit);
							}
						}
						CombatantArray.Add(NewCombatUnit);
					}
					NewCampaignData.Combatant = CombatantArray;	
				}
			}
		}

	} while (term);

	LoadZones(CampaignPath);
	LoadMissionList(CampaignPath);
	LoadTemplateList(CampaignPath);
	LoadMission(CampaignPath);
	NewCampaignData.Zone = ZoneArray;
	NewCampaignData.MissionList = MissionListArray;
	NewCampaignData.TemplateList = TemplateListArray;
	NewCampaignData.Missions = MissionArray;

	// define our data table struct
	FName RowName = FName(FString(name));
	CampaignDataTable->AddRow(RowName, NewCampaignData);
	CampaignData = NewCampaignData;

	SSWInstance->loader->ReleaseBuffer(block);
}

void AGameDataLoader::LoadZones(FString Path)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadZones()"));
	
	FString FileName = Path;
	FileName.Append("zones.def");

	UE_LOG(LogTemp, Log, TEXT("Loading Campaign Zone: %s"), *FileName);

	const char* fn = TCHAR_TO_ANSI(*FileName);

	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(fn);

	BYTE* block = 0;

	SSWInstance->loader->LoadBuffer(fn, block, true);

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	ZoneArray.Empty();

	if (!term) {
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "ZONES") {
			UE_LOG(LogTemp, Log, TEXT("Invalid Zone File %s"), *FString(fn));
			return;
		}
	}

	do {
		delete term;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def->name()->value() == "zone") {
				if (!def->term() || !def->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("WARNING: zone struct missing in '%s'"), *FString(filename));
				}
				else {
					TermStruct* val = def->term()->isStruct();

					FS_CampaignZone NewCampaignZone;

					for (int i = 0; i < val->elements()->size(); i++) {
						TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef) {
							if (pdef->name()->value() == "region") {
								GetDefText(ZoneRegion, pdef, fn);
								NewCampaignZone.Region = FString(ZoneRegion);
								//zone->AddRegion(rgn);
							}
							else if (pdef->name()->value() == "system") {
								GetDefText(ZoneSystem, pdef, fn);
								NewCampaignZone.System = FString(ZoneSystem);
								//zone->system = rgn;
							}
						}	
					}
					ZoneArray.Add(NewCampaignZone);
				}
			}
		}

	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
}

void
AGameDataLoader::LoadMissionList(FString Path)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadMissionList()"));
	MissionListArray.Empty();

	FString FileName = Path;
	FileName.Append("Missions.def");

	UE_LOG(LogTemp, Log, TEXT("Loading Mission List : %s"), *FileName);

	const char* fn = TCHAR_TO_ANSI(*FileName);

	if(FFileManagerGeneric::Get().FileExists(*FileName) == false)
	{ 
		UE_LOG(LogTemp, Log, TEXT("Mission List does not exist"));
		return;
	}

	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(fn);

	BYTE* block = 0;

	SSWInstance->loader->LoadBuffer(fn, block, true);

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	if (!term) {
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "MISSIONLIST") {
			UE_LOG(LogTemp, Log, TEXT("WARNING: invalid mission list file '%s'"), *FString(fn));
			return;
		}
	}

	do {
		delete term; term = 0;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def->name()->value() == "mission") {
				if (!def->term() || !def->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("WARNING: zone struct missing in '%s'"), *FString(fn));
				}
				else {
					TermStruct* val = def->term()->isStruct();

					FS_CampaignMissionList NewMissionList;

					int   Id = 0;
					Text  Name;
					Text  Desc;
					Text  Script;
					Text  System = "Unknown";
					Text  Region = "Unknown";
					int   Start = 0;
					int   Type = 0;

					for (int i = 0; i < val->elements()->size(); i++) {
						TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef->name()->value() == "id") {
							GetDefNumber(Id, pdef, fn);
							NewMissionList.Id = Id;
						}
						else if (pdef->name()->value() == "name") {
							GetDefText(Name, pdef, fn);
							NewMissionList.Name = FString(Name);
						}
						else if (pdef->name()->value() == "desc") {
							GetDefText(Desc, pdef, fn);
							NewMissionList.Description = FString(Desc);
						}
						else if (pdef->name()->value() == "start") {
							GetDefTime(Start, pdef, fn);
							NewMissionList.Description = FString(Desc);
						}
						else if (pdef->name()->value() == "system") {
							GetDefText(System, pdef, fn);
							NewMissionList.System = FString(System);
						}
						else if (pdef->name()->value() == "region") {
							GetDefText(Region, pdef, fn);
							NewMissionList.Region = FString(Region);
						}
						else if (pdef->name()->value() == "script") {
							GetDefText(Script, pdef, fn);
							NewMissionList.Script = FString(Script);
						}
						else if (pdef->name()->value() == "type") {
							char typestr[64];
							GetDefText(typestr, pdef, fn);
							Type = Mission::TypeFromName(typestr);
							NewMissionList.Type = Type;
						}
					}
					MissionListArray.Add(NewMissionList);
				}
			}
		}
	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
}

void
AGameDataLoader::LoadTemplateList(FString Path)
{
	
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadTemplateList()"));

	FString FileName = Path;
	FileName.Append("Templates.def");

	UE_LOG(LogTemp, Log, TEXT("Loading Template List : %s"), *FileName);

	const char* fn = TCHAR_TO_ANSI(*FileName);

	if (FFileManagerGeneric::Get().FileExists(*FileName) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("Template List does not exist"));
		return;
	}

	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(fn);

	BYTE* block = 0;

	SSWInstance->loader->LoadBuffer(fn, block, true);

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	if (!term) {
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "TEMPLATELIST") {
			UE_LOG(LogTemp, Log, TEXT("WARNING: invalid template list file '%s'"), *FString(fn));
			return;
		}
	}

	do {
		delete term; 
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def->name()->value() == "mission") {
				if (!def->term() || !def->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("WARNING: mission struct missing in '%s'"), *FString(fn));
				}
				else {
					TermStruct* val = def->term()->isStruct();
					
					FS_CampaignTemplateList NewTemplateList;

					Text  Name = "";
					Text  Script = "";
					Text  Region = "";
					int   id = 0;
					int   msn_type = 0;
					int   grp_type = 0;

					int   min_rank = 0;
					int   max_rank = 0;
					int   action_id = 0;
					int   action_status = 0;
					int   exec_once = 0;
					int   start_before = Game::TIME_NEVER;
					int   start_after = 0;

					for (int i = 0; i < val->elements()->size(); i++) {
					TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef->name()->value() == "id") {
							GetDefNumber(id, pdef, fn);
							NewTemplateList.Id = id;
						}
						else if (pdef->name()->value() == "name") {
							GetDefText(Name, pdef, fn);
							NewTemplateList.Name = FString(Name);
						}
						else if (pdef->name()->value() == "script") {
							GetDefText(Script, pdef, fn);
							NewTemplateList.Script = FString(Script);
						}
						else if (pdef->name()->value() == "rgn" || pdef->name()->value() == "region") {
							GetDefText(Region, pdef, fn);
							NewTemplateList.Region = FString(Region);
						}
						else if (pdef->name()->value() == "type") {
							char typestr[64];
							GetDefText(typestr, pdef, fn);
							msn_type = Mission::TypeFromName(typestr);
							NewTemplateList.MissionType = msn_type;
						}

						else if (pdef->name()->value() == "group") {
							char typestr[64];
							GetDefText(typestr, pdef, fn);
							grp_type = CombatGroup::TypeFromName(typestr);
							NewTemplateList.GroupType = grp_type;
						}

						else if (pdef->name()->value() == "min_rank") {
							GetDefNumber(min_rank, pdef, fn);
							NewTemplateList.MinRank = min_rank;
						}
						else if (pdef->name()->value() == "max_rank") {
							GetDefNumber(max_rank, pdef, fn);
							NewTemplateList.MaxRank = max_rank;
						}
						else if (pdef->name()->value() == "action_id") {
							GetDefNumber(action_id, pdef, fn);
							NewTemplateList.ActionId = action_id;
						}
						else if (pdef->name()->value() == "action_status") {
							GetDefNumber(action_status, pdef, fn);
							NewTemplateList.ActionStatus = action_status;
						}
						else if (pdef->name()->value() == "exec_once") {
							GetDefNumber(exec_once, pdef, fn);
							NewTemplateList.ExecOnce = exec_once;
						}
						else if (pdef->name()->value().contains("before")) {
							if (pdef->term()->isNumber()) {
								GetDefNumber(start_before, pdef, fn);
								NewTemplateList.StartBefore = start_before;
							}
							else {
								GetDefTime(start_before, pdef, fn);
								start_before -= Game::ONE_DAY;
								NewTemplateList.StartBefore = start_before;
							}
						}
						else if (pdef->name()->value().contains("after")) {
							if (pdef->term()->isNumber()) {
								GetDefNumber(start_after, pdef, fn);
								NewTemplateList.StartAfter = start_after;
							}
							else {
								GetDefTime(start_after, pdef, fn);
								start_after -= Game::ONE_DAY;
								NewTemplateList.StartAfter = start_after;
							}
						}
					}
					TemplateListArray.Add(NewTemplateList);
				}
			}
		}
	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
}

void AGameDataLoader::LoadMission(FString Path)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadMission()"));
	FString PathName = CampaignPath;
	PathName.Append("Scenes/");
	
	TArray<FString> output;
	output.Empty();

	FString file = PathName + "*.def";
	FFileManagerGeneric::Get().FindFiles(output, *file, true, false);

	for (int i = 0; i < output.Num(); i++) {

		FString FileName = PathName;
		FileName.Append(output[i]);

		char* fn = TCHAR_TO_ANSI(*FileName);

		ParseMission(fn);
	}
}


void
AGameDataLoader::ParseMission(const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseMission()"));

	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(fn);

	BYTE* block = 0;
	SSWInstance->loader->LoadBuffer(fn, block, true);

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();
	
	FString fs = FString(ANSI_TO_TCHAR(fn));
	FString FileString;

	if (FFileHelper::LoadFileToString(FileString, *fs, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);

		const char* result = TCHAR_TO_ANSI(*FileString);
	}
	if (!term) {
		UE_LOG(LogTemp, Log, TEXT("WARNING: could not parse '%s'"), *FString(fn));
		return;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("MISSION file '%s'"), *FString(fn));
	}

	/*if (!term) {
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "MISSION") {
			UE_LOG(LogTemp, Log, TEXT("WARNING: invalid MISSION file '%s'"), *FString(fn));
			return;
		}
	}*/

	FS_CampaignMission NewMission;

	int					id = 0;

	Text				Region = "";
	Text				Scene = "";
	Text                System = "";
	Text                Subtitles = "";
	Text                Name = "";
	Text                Desc = "";
	Text				TargetName = "";
	Text				WardName = "";
	Text                Objective = "";
	Text                Sitrep = "";

	int                 Type = 0;
	int                 Team = 0;
	Text                Start = ""; // time

	double              Stardate = 0;

	bool                Degrees = false;

	do {
		delete term;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "name") {
					GetDefText(Name, def, filename);
					NewMission.Name = FString(Name);
					UE_LOG(LogTemp, Log, TEXT("mission name '%s'"), *FString(Name));
				}
		
				else if (def->name()->value() == "scene") {
					GetDefText(Scene, def, fn);
					NewMission.Scene = FString(Scene);

				}
				else if (def->name()->value() == "desc") {
					GetDefText(Desc, def, fn);
					if (Desc.length() > 0 && Desc.length() < 32) {
						NewMission.Desc = FString(Desc);
					}
				}
				else if (def->name()->value() == "type") {
					char typestr[64];
					GetDefText(typestr, def, fn);
					Type = Mission::TypeFromName(typestr);
					NewMission.Type = Type;
				}
				else if (def->name()->value() == "system") {
					GetDefText(System, def, fn);
					NewMission.System = FString(System);
				}
				else if (def->name()->value() == "region") {
					GetDefText(Region, def, fn);
					NewMission.Region = FString(Region);
				}
				else if (def->name()->value() == "degrees") {
					GetDefBool(Degrees, def, fn);
					NewMission.Degrees = Degrees;
				}
				else if (def->name()->value() == "objective") {
					GetDefText(Objective, def, fn);
					if (Objective.length() > 0 && Objective.length() < 32) {
						NewMission.Objective = FString(Objective);
					}
				}
				else if (def->name()->value() == "sitrep") {
					GetDefText(Sitrep, def, fn);
					if (Sitrep.length() > 0 && Sitrep.length() < 32) {
						NewMission.Sitrep = FString(Sitrep);
					}
				}
				else if (def->name()->value() == "subtitles") {
					GetDefText(Subtitles, def, fn);
					NewMission.Subtitles = FString(Subtitles);
				
				}
				else if (def->name()->value() == "start") {
					GetDefText(Start, def, fn);
					NewMission.StartTime = FString(Start);
					//GetDefTime(start, def, fn);

				}
				else if (def->name()->value() == "stardate") {
					GetDefNumber(Stardate, def, fn);
					NewMission.Stardate = Stardate;
				}
				else if (def->name()->value() == "team") {
					GetDefNumber(Team, def, fn);
					NewMission.Team = Team;
				}
				else if (def->name()->value() == "target") {
					GetDefText(TargetName, def, fn);
					NewMission.TargetName = FString(TargetName);
				}
				else if (def->name()->value() == "ward") {
					GetDefText(WardName, def, filename);
					NewMission.WardName = FString(WardName);
				}
				else if ((def->name()->value() == "element") ||
					(def->name()->value() == "ship") ||
					(def->name()->value() == "station")) {

					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("ERROR: element struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//MissionElement* elem = ParseElement(val);
						//AddElement(elem);
					}
				}

				else if (def->name()->value() == "event") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("ERROR: event struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//MissionEvent* event = ParseEvent(val);
						//AddEvent(event);
					}
				}
			}
		}        // term
	} while (term); 

	MissionArray.Add(NewMission);

}

void
AGameDataLoader::ParseElement(TermStruct* val, const char* fn)
{
	Text  Name = "";
	Text  Carrier = "";
	Text  Commander = "";
	Text  Squadron = "";
	Text  Path = "";
	Text  Design = "";
	Text  SkinName = "";
	Text  RoleName = "";
	Text  RegionName = "";
	Text  Instr = "";

	Vec3  Loc(0.0f, 0.0f, 0.0f);

	int   Deck = 1;
	int   IFFCode = 0;
	int   Count = 1;
	int   MaintCount = 0;
	int   DeadCount = 0;
	int   Player = 0;
	int   CommandAI = 0;
	int   Respawns = 0;
	int   HoldTime = 0;
	int   ZoneLock = 0;

	bool Alert = false;
	bool Playable = false;
	bool Rogue = false;
	bool Invulnerable = false;
	

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {

			if (pdef->name()->value() == "name") {
				GetDefText(Name, pdef, fn);
			} 
			else if (pdef->name()->value() == "carrier") {
				GetDefText(Carrier, pdef, fn);
			}
			else if (pdef->name()->value() == "commander") {
				GetDefText(Commander, pdef, fn);
			}
			else if (pdef->name()->value() == "squadron") {
				GetDefText(Squadron, pdef, fn);
			}
			else if (pdef->name()->value() == "path") {
				GetDefText(Path, pdef, fn);
			}
			else if (pdef->name()->value() == "design") {
				GetDefText(Design, pdef, fn);
		
			}
			else if (pdef->name()->value() == "skin") {
					GetDefText(SkinName, pdef, fn);
			}
			else if (pdef->name()->value() == "mission") {
				GetDefText(RoleName, pdef, fn);

			}
			else if (pdef->name()->value() == "intel") {
				GetDefText(RoleName, pdef, fn);
		
			}

			else if (pdef->name()->value() == "loc") {
				GetDefVec(Loc, pdef, fn);

			}

			else if (pdef->name()->value() == "rloc") {
				if (pdef->term()->isStruct()) {
					
					//RLoc* rloc = ParseRLoc(pdef->term()->isStruct());
				}
			}

			else if (pdef->name()->value().indexOf("head") == 0) {
				/*if (pdef->term()->isArray()) {
					Vec3 head;
					GetDefVec(head, pdef, filename);
					if (degrees) head.z *= (float)DEGREES;
					element->heading = head.z;
				}
				else if (pdef->term()->isNumber()) {
					double heading = 0;
					GetDefNumber(heading, pdef, filename);
					if (degrees) heading *= DEGREES;
					element->heading = heading;
				}*/
			}

			else if (pdef->name()->value() == "region" || pdef->name()->value() == "rgn") {
				GetDefText(RegionName, pdef, fn);
			}

			else if (pdef->name()->value() == "iff") {
				GetDefNumber(IFFCode, pdef, fn);
			
			}
			else if (pdef->name()->value() == "count") {
				GetDefNumber(Count, pdef, fn);
			
			}
			else if (pdef->name()->value() == "maint_count") {
				GetDefNumber(MaintCount, pdef, fn);
			}
			else if (pdef->name()->value() == "dead_count") {
				GetDefNumber(DeadCount, pdef, fn);
			}
			else if (pdef->name()->value() == "player") {
				GetDefNumber(Player, pdef, fn);
			}	
			else if (pdef->name()->value() == "alert") {
				GetDefBool(Alert, pdef, fn);
			}
			else if (pdef->name()->value() == "playable") {
				GetDefBool(Playable, pdef, fn); 
			}
			else if (pdef->name()->value() == "rogue") {
				GetDefBool(Rogue, pdef, fn);
			}
			else if (pdef->name()->value() == "invulnerable") {
				GetDefBool(Invulnerable, pdef, fn);
			}
			else if (pdef->name()->value() == "command_ai") {
				GetDefNumber(CommandAI, pdef, fn);
			}
			else if (pdef->name()->value().indexOf("respawn") == 0) {
				GetDefNumber(Respawns, pdef, fn);
			}
			else if (pdef->name()->value().indexOf("hold") == 0) {
				GetDefNumber(HoldTime, pdef, fn);
			}
			else if (pdef->name()->value().indexOf("zone") == 0) {
				if (pdef->term() && pdef->term()->isBool()) {
					bool locked = false;
					GetDefBool(locked, pdef, fn);
				}
				else {
					GetDefNumber(ZoneLock, pdef, fn);
				}
			}

			else if (pdef->name()->value() == "objective") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("Mission error - No objective"));
				}
				else {
					TermStruct* val = pdef->term()->isStruct();
					//Instruction* obj = ParseInstruction(val, element);
				}
			}
			else if (pdef->name()->value() == "instr") {
				GetDefText(Instr, pdef, fn);

			}

			else if (pdef->name()->value() == "ship") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("Mission error - no ship"));
				}
				else {
					TermStruct* val = pdef->term()->isStruct();
					//MissionShip* s = ParseShip(val, element);
					
				}
			}

			else if (pdef->name()->value() == "order" || pdef->name()->value() == "navpt") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("Mission error - no navpt"));
		
				}
				else {
					TermStruct* val = pdef->term()->isStruct();
					//Instruction* npt = ParseInstruction(val, element);
					//element->navlist.append(npt);
				}
			}

			else if (pdef->name()->value() == "loadout") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("Mission error - no loadout"));
				
				}
				else {
					TermStruct* val = pdef->term()->isStruct();
					//ParseLoadout(val, element);
				}
			}
		}
	}

}
// +--------------------------------------------------------------------+

void
AGameDataLoader::ParseAction(TermStruct* val, const char* fn)
{
	if (!val) {
		UE_LOG(LogTemp, Log, TEXT("invalid action in campaign '%s'"), *FString(name.data()));
		return;
	}

	int   id = 0;
	int   type = 0;
	int   subtype = 0;
	int   opp_type = -1;
	int   team = 0;
	int   source = 0;
	Vec3  loc(0.0f, 0.0f, 0.0f);
	Text  system;
	Text  region;
	Text  file;
	Text  image;
	Text  scene;
	Text  text;
	int   count = 1;
	int   start_before = Game::TIME_NEVER;
	int   start_after = 0;
	int   min_rank = 0;
	int   max_rank = 100;
	int   delay = 0;
	int   probability = 100;

	int   asset_type = 0;
	int   asset_id = 0;
	int   target_type = 0;
	int   target_id = 0;
	int   target_iff = 0;

	//CombatAction* action = 0;


	for (int i = 0; i < val->elements()->size(); i++) {
		
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "id") {
				GetDefNumber(id, pdef, fn);
				NewCampaignAction.Id = id;
				UE_LOG(LogTemp, Log, TEXT("action id: '%d'"), id);
			}
			else if (pdef->name()->value() == "type") {
				char txt[64];
				GetDefText(txt, pdef, fn);
				type = CombatAction::TypeFromName(txt);
				NewCampaignAction.Type = FString(txt);
				UE_LOG(LogTemp, Log, TEXT("action type: '%s'"), *FString(txt));
			}
			else if (pdef->name()->value() == "subtype") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(subtype, pdef, fn);
					NewCampaignAction.Subtype = subtype;
				}

				else if (pdef->term()->isText()) {
					char txt[64];
					GetDefText(txt, pdef, fn);

					if (type == CombatAction::MISSION_TEMPLATE) {
						subtype = Mission::TypeFromName(txt);
					}
					else if (type == CombatAction::COMBAT_EVENT) {
						subtype = CombatEvent::TypeFromName(txt);
					}
					else if (type == CombatAction::INTEL_EVENT) {
						//subtype = Intel::IntelFromName(txt);
					}
					//NewCampaignAction.Subtype = subtype;
				}
				
			}
			else if (pdef->name()->value() == "opp_type") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(opp_type, pdef, fn);
					//NewCampaignAction.OppType = opp_type;
				}

				else if (pdef->term()->isText()) {
					char txt[64];
					GetDefText(txt, pdef, fn);

					if (type == CombatAction::MISSION_TEMPLATE) {
						opp_type = Mission::TypeFromName(txt);
					}
				}
			}
			else if (pdef->name()->value() == "source") {
				char txt[64];
				GetDefText(txt, pdef, fn);
				//source = CombatEvent::SourceFromName(txt);
				//NewCampaignAction.Source = txt;
			}
			else if (pdef->name()->value() == "team") {
				GetDefNumber(team, pdef, fn);
				//NewCampaignAction.Team = team;
			}
			else if (pdef->name()->value() == "iff") {
				GetDefNumber(team, pdef, fn);
				//NewCampaignAction.Iff = team;
			}
			else if (pdef->name()->value() == "count") {
				GetDefNumber(count, pdef, fn);
				//NewCampaignAction.Count = count;
			}
			else if (pdef->name()->value().contains("before")) {
				if (pdef->term()->isNumber()) {
					GetDefNumber(start_before, pdef, fn);
					//NewCampaignAction.StartBefore = start_before;
				}
				else {
					GetDefTime(start_before, pdef, fn);
					start_before -= Game::ONE_DAY;
					//NewCampaignAction.StartBefore = start_before;
				}
			}
			else if (pdef->name()->value().contains("after")) {
				if (pdef->term()->isNumber()) {
					GetDefNumber(start_after, pdef, fn);
					//NewCampaignAction.StartAfter = start_after;
				}
				else {
					GetDefTime(start_after, pdef, fn);
					start_after -= Game::ONE_DAY;
					//NewCampaignAction.StartAfter = start_after;
				}
			}
			else if (pdef->name()->value() == "min_rank") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(min_rank, pdef, fn);
					//NewCampaignAction.MinRank = min_rank;
				}
				else {
					char rank_name[64];
					GetDefText(rank_name, pdef, fn);
					min_rank = PlayerData::RankFromName(rank_name);
					//NewCampaignAction.MinRank = min_rank;
				}
			}
			else if (pdef->name()->value() == "max_rank") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(max_rank, pdef, fn);
					//NewCampaignAction.MaxRank = max_rank;
				}
				else {
					char rank_name[64];
					GetDefText(rank_name, pdef, fn);
					max_rank = PlayerData::RankFromName(rank_name);
					//NewCampaignAction.MaxRank = max_rank;
				}
			}
			else if (pdef->name()->value() == "delay") {
				GetDefNumber(delay, pdef, fn);
				//NewCampaignAction.Delay = delay;
			}
			else if (pdef->name()->value() == "probability") {
				GetDefNumber(probability, pdef, fn);
				//NewCampaignAction.Probability = probability;
			}
			else if (pdef->name()->value() == "asset_type") {
				char type_name[64];
				GetDefText(type_name, pdef, fn);
				//asset_type = CombatGroup::TypeFromName(type_name);
				//NewCampaignAction.AssetType = type_name;
			}
			else if (pdef->name()->value() == "target_type") {
				char type_name[64];
				GetDefText(type_name, pdef, fn);
				//NewCampaignAction.TargetType = type_name;
				//target_type = CombatGroup::TypeFromName(type_name);
			}
			else if (pdef->name()->value() == "location" ||
				pdef->name()->value() == "loc") {
				GetDefVec(loc, pdef, fn);
				//NewCampaignAction.Location.X = loc.x;
				//NewCampaignAction.Location.Y = loc.y;
				//NewCampaignAction.Location.Z = loc.z;

			}
			else if (pdef->name()->value() == "system" ||
				pdef->name()->value() == "sys") {
				GetDefText(system, pdef, fn);
				//NewCampaignAction.System = FString(system);
			}
			else if (pdef->name()->value() == "region" ||
				pdef->name()->value() == "rgn" ||
				pdef->name()->value() == "zone") {
				GetDefText(region, pdef, fn);
				//NewCampaignAction.Region = FString(region);
			}
			else if (pdef->name()->value() == "file") {
				GetDefText(file, pdef, fn);
				//NewCampaignAction.File = FString(file);
			}
			else if (pdef->name()->value() == "image") {
				GetDefText(image, pdef, fn);
				//NewCampaignAction.Image = FString(image);
			}
			else if (pdef->name()->value() == "scene") {
				GetDefText(scene, pdef, fn);
				//NewCampaignAction.Scene = FString(scene);
			}
			else if (pdef->name()->value() == "text") {
				GetDefText(text, pdef, fn);
				text = Game::GetText(text);
				//NewCampaignAction.Text = FString(text);
			}
			else if (pdef->name()->value() == "asset_id") {
				GetDefNumber(asset_id, pdef, fn);
				//NewCampaignAction.AssetId = asset_id;
			}
			else if (pdef->name()->value() == "target_id") {
				GetDefNumber(target_id, pdef, fn);
				//NewCampaignAction.TargetId = target_id;
			}
				
			else if (pdef->name()->value() == "target_iff") {
				GetDefNumber(target_iff, pdef, fn);
				//NewCampaignAction.TargetIff = target_iff;
			}
		}
		
		CampaignActionArray.Add(NewCampaignAction);

			/*else if (pdef->name()->value() == "asset_kill") {
				if (!action)
					action = new CombatAction(id, type, subtype, team);

				if (action) {
					char txt[64];
					GetDefText(txt, pdef, fn);
					action->AssetKills().append(new Text(txt));
				}
			}

			else if (pdef->name()->value() == "target_kill") {
				if (!action)
					action = new CombatAction(id, type, subtype, team);

				if (action) {
					char txt[64];
					GetDefText(txt, pdef, fn);
					action->TargetKills().append(new Text(txt));
				}
			}

			else if (pdef->name()->value() == "req") {
				if (!action)
					action = new CombatAction(id, type, subtype, team);

				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("WARNING: action req struct missing in '%s'"), *FString(fn)); 

				}
				else if (action) {
					TermStruct* val2 = pdef->term()->isStruct();

					int  act = 0;
					int  stat = CombatAction::COMPLETE;
					bool not_action = false;

					Combatant* c1 = 0;
					Combatant* c2 = 0;
					int         comp = 0;
					int         score = 0;
					int         intel = 0;
					int         gtype = 0;
					int         gid = 0;

					for (int index = 0; index < val2->elements()->size(); index++) {
						TermDef* pdef2 = val2->elements()->at(i)->isDef();
						if (pdef2) {
							if (pdef2->name()->value() == "action") {
								GetDefNumber(act, pdef2, filename);
							}
							else if (pdef2->name()->value() == "status") {
								char txt[64];
								GetDefText(txt, pdef2, filename);
								stat = CombatAction::StatusFromName(txt);
							}
							else if (pdef2->name()->value() == "not") {
								GetDefBool(not_action, pdef2, filename);
							}

							else if (pdef2->name()->value() == "c1") {
								char txt[64];
								GetDefText(txt, pdef2, filename);
								c1 = GetCombatant(txt);
							}
							else if (pdef2->name()->value() == "c2") {
								char txt[64];
								GetDefText(txt, pdef2, filename);
								c2 = GetCombatant(txt);
							}
							else if (pdef2->name()->value() == "comp") {
								char txt[64];
								GetDefText(txt, pdef2, filename);
								comp = CombatActionReq::CompFromName(txt);
							}
							else if (pdef2->name()->value() == "score") {
								GetDefNumber(score, pdef2, filename);
							}
							else if (pdef2->name()->value() == "intel") {
								if (pdef2->term()->isNumber()) {
									GetDefNumber(intel, pdef2, filename);
								}
								else if (pdef2->term()->isText()) {
									char txt[64];
									GetDefText(txt, pdef2, filename);
									//intel = Intel::IntelFromName(txt);
								}
							}
							else if (pdef2->name()->value() == "group_type") {
								char type_name[64];
								GetDefText(type_name, pdef2, filename);
								gtype = CombatGroup::TypeFromName(type_name);
							}
							else if (pdef2->name()->value() == "group_id") {
								GetDefNumber(gid, pdef2, filename);
							}
						}
					}

					if (act)
						action->AddRequirement(act, stat, not_action);

					else if (gtype)
						action->AddRequirement(c1, gtype, gid, comp, score, intel);

					else
						action->AddRequirement(c1, c2, comp, score);
				}
			}*/
	}
		
}

// +--------------------------------------------------------------------+

CombatGroup*
AGameDataLoader::CloneOver(CombatGroup* force, CombatGroup* clone, CombatGroup* group)
{
	CombatGroup* orig_parent = group->GetParent();

	if (orig_parent) {
		CombatGroup* clone_parent = clone->FindGroup(orig_parent->Type(), orig_parent->GetID());

		if (!clone_parent)
			clone_parent = CloneOver(force, clone, orig_parent);

		CombatGroup* new_clone = clone->FindGroup(group->Type(), group->GetID());

		if (!new_clone) {
			new_clone = group->Clone(false);
			clone_parent->AddComponent(new_clone);
		}

		return new_clone;
	}
	else {
		return clone;
	}
}

// +--------------------------------------------------------------------+

void
AGameDataLoader::Unload()
{
	SetStatus(CAMPAIGN_INIT);

	Game::ResetGameTime();
	AStarSystem::SetBaseTime(0);

	startTime = Stardate();
	loadTime = startTime;
	lockout = 0;

	//for (int i = 0; i < NUM_IMAGES; i++)
	//	image[i].ClearImage();

	Clear();

	//zones.destroy();
}

// +--------------------------------------------------------------------+

void
AGameDataLoader::SetStatus(int s)
{
	status = s;

	// record the win in player profile:
	if (status == CAMPAIGN_SUCCESS) {
		PlayerData* player = PlayerData::GetCurrentPlayer();

		if (player)
			player->SetCampaignComplete(campaign_id);
	}

	if (status > CAMPAIGN_ACTIVE) {
		::Print("Campaign::SetStatus() destroying mission list at campaign end\n");
		//missions.destroy();
	}
}

double
AGameDataLoader::Stardate()
{
	return AStarSystem::GetStardate();
}

void
AGameDataLoader::Clear()
{
	//missions.destroy();
	//planners.destroy();
	//combatants.destroy();
	//events.destroy();
	//actions.destroy();

	player_group = 0;
	player_unit = 0;

	updateTime = time;
}

Combatant*
AGameDataLoader::GetCombatant(const char* cname)
{
	ListIter<Combatant> iter = combatants;
	while (++iter) {
		Combatant* c = iter.value();
		if (!strcmp(c->Name(), cname))
			return c;
	}

	return 0;
}
