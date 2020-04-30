class SchanaPartyManagerServer
{
	private ref map<ref string, ref set<ref string>> configurations;
	private bool canSendInfo = true;
	private bool schanaDebug = false;

	void SchanaPartyManagerServer()
	{
		Print("[SchanaParty] Server Init " + MissionBase.SCHANA_PARTY_VERSION);
		configurations = new ref map<ref string, ref set<ref string>>();
		GetRPCManager().AddRPC("SchanaModParty", "ServerRegisterPartyRPC", this, SingleplayerExecutionType.Both);

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.SendInfo, 10000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.ResetSendInfoLock, 1000, true);
	}

	void ~SchanaPartyManagerServer()
	{
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.SendInfo);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.ResetSendInfoLock);
	}

	void ResetSendInfoLock()
	{
		canSendInfo = true;
	}

	void ServerRegisterPartyRPC(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		Param2<ref string, ref array<ref string>> data;
		if (!ctx.Read(data))
			return;

		if (schanaDebug)
		{
			string result;
	        JsonSerializer().WriteToString(data, false, result);
			SchanaPartyUtils.LogMessage("ServerRegisterPartyRPC " + result);
		}

		ServerRegisterParty(data.param1, data.param2);
	}

	void ServerRegisterParty(string key, ref array<ref string> ids)
	{
		Print("[SchanaParty] Register " + ids.Count().ToString() + " to " + key);
		auto party_members = new ref set<ref string>();
		foreach (string id : ids)
		{
			party_members.Insert(id);
		}
		
		string result;
		
		if (schanaDebug)
		{
	        JsonSerializer().WriteToString(configurations, false, result);
			SchanaPartyUtils.LogMessage("ServerRegisterParty Before " + result);
		}
		
		configurations.Set(key, party_members);
		
		if (schanaDebug)
		{
			JsonSerializer().WriteToString(configurations, false, result);
			SchanaPartyUtils.LogMessage("ServerRegisterParty After " + result);
		}
		
		SendInfo();
	}

	private ref map<ref string, ref set<ref string>> GetParties()
	{
		auto parties = new ref map<ref string, ref set<ref string>>();

		foreach (auto owner_id, auto party_ids : configurations)
		{
			auto validated_party_ids = new ref set<ref string>();

			foreach (string member_id : party_ids)
			{
				if (configurations.Get(member_id) && configurations.Get(member_id).Find(owner_id) != -1)
				{
					validated_party_ids.Insert(member_id);
				}
			}
			
			parties.Insert(owner_id, validated_party_ids);
		}

		return parties;
	}

	private ref map<ref string, ref vector> GetPositions()
	{
		auto positions = new ref map<ref string, ref vector>();

		MissionServer mission = MissionServer.Cast(GetGame().GetMission());

		foreach (Man man : mission.m_Players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (player && player.GetIdentity())
			{
				positions.Insert(player.GetIdentity().GetId(), player.GetPosition());
			}
		}

		if (schanaDebug)
		{
			positions.Insert("a", "4500 350 10000" + Vector(Math.RandomFloat(-100, 100), 0, 0));
			positions.Insert("b", "4500 100 12000" + Vector(Math.RandomFloat(-100, 100), 0, 0));
			positions.Insert("c", "4500 200 8000" + Vector(Math.RandomFloat(-100, 100), 0, 0));
		}
		
		return positions;
	}

	private ref map<ref string, ref float> GetHealths()
	{
		auto healths = new ref map<ref string, ref float>();

		MissionServer mission = MissionServer.Cast(GetGame().GetMission());

		foreach (Man man : mission.m_Players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (player && player.GetIdentity())
			{
				healths.Insert(player.GetIdentity().GetId(), player.GetHealth("", ""));
			}
		}

		if (schanaDebug)
		{
			healths.Insert("a", 100);
			healths.Insert("b", 10);
			healths.Insert("c", 42);
		}

		return healths;
	}

	void SendInfo()
	{
		if (canSendInfo)
		{
			Print("[SchanaParty] SendInfo");
			auto id_map = new ref map<ref string, ref PlayerBase>();

			MissionServer mission = MissionServer.Cast(GetGame().GetMission());

			foreach (Man man : mission.m_Players)
			{
				PlayerBase player = PlayerBase.Cast(man);
				if (player && player.GetIdentity())
				{
					id_map.Insert(player.GetIdentity().GetId(), player);
				}
			}

			SendPartyInfo(id_map);
			SendPlayersInfo(id_map);
			
			canSendInfo = false;
		}
	}

	void SendPartyInfo(ref map<ref string, ref PlayerBase> id_map)
	{
		string result;
		auto positions = GetPositions();
		auto server_healths = GetHealths();
		auto parties = GetParties();
		
		JsonSerializer().WriteToString(positions, false, result);
		SchanaPartyUtils.LogMessage("Positions " + result);
		JsonSerializer().WriteToString(server_healths, false, result);
		SchanaPartyUtils.LogMessage("Healths " + result);
		JsonSerializer().WriteToString(parties, false, result);
		SchanaPartyUtils.LogMessage("Parties " + result);
		
		foreach (auto id, auto party_ids : parties)
		{
			if (!positions.Get(id))
			{
				configurations.Remove(id);
			}
			else
			{
				auto ids = new ref array<ref string>;
				auto locations = new ref array<ref vector>;
				auto healths = new ref array<ref float>;
				foreach (string party_id : party_ids)
				{
					if (positions.Contains(party_id))
					{
						ids.Insert(party_id);
						locations.Insert(positions.Get(party_id));
						healths.Insert(server_healths.Get(party_id));
					}
				}
				auto info = new Param3<ref array<ref string>, ref array<ref vector>, ref array<ref float>>(ids, locations, healths);
				
        		JsonSerializer().WriteToString(info, false, result);
				SchanaPartyUtils.LogMessage("SendInfo to " + id + " " + result);

				if (!schanaDebug || (id != "a" && id != "b" && id != "c"))
				{
					GetRPCManager().SendRPC("SchanaModParty", "ClientUpdatePartyInfoRPC", info, false, id_map.Get(id).GetIdentity());
				}
			}
		}
	}

	void SendPlayersInfo(ref map<ref string, ref PlayerBase> id_map)
	{
		auto all_player_ids = new ref array<ref string>;
		auto all_player_names = new ref array<ref string>;
		foreach (auto player_id, auto player_base_player : id_map)
		{
			all_player_ids.Insert(player_id);
			all_player_names.Insert(player_base_player.GetIdentity().GetName());
		}

		if (schanaDebug)
		{
			all_player_ids.Insert("a");
			all_player_ids.Insert("b");
			all_player_ids.Insert("c");
			all_player_names.Insert("Schana");
			all_player_names.Insert("CarimSurvivor");
			all_player_names.Insert("cnofafva");
			for(int i=10; i<15; ++i)
			{
				all_player_ids.Insert("id" + i.ToString());
				all_player_names.Insert("player " + i.ToString());
			}
		}

		auto all_player_info = new Param2<ref array<ref string>, ref array<ref string>>(all_player_ids, all_player_names);

		string result;
        JsonSerializer().WriteToString(all_player_info, false, result);
		SchanaPartyUtils.LogMessage("SendPlayers " + result);

		GetRPCManager().SendRPC("SchanaModParty", "ClientUpdatePlayersInfoRPC", all_player_info);
	}
}

static ref SchanaPartyManagerServer g_SchanaPartyManagerServer;
static ref SchanaPartyManagerServer GetSchanaPartyManagerServer()
{
	if (g_Game.IsServer() && !g_SchanaPartyManagerServer)
	{
		g_SchanaPartyManagerServer = new SchanaPartyManagerServer;
	}
	return g_SchanaPartyManagerServer;
}
