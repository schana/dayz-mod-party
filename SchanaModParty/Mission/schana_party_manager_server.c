class SchanaPartyManagerServer
{
    private ref map<string, ref set<string>> configurations;

    void SchanaPartyManagerServer()
    {
        Print("[SchanaParty] Server Init");
		configurations = new ref map<string, ref set<string>>();
        GetRPCManager().AddRPC("SchanaModParty", "ServerRegisterPartyRPC", this, SingleplayerExecutionType.Both);

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.SendInfo, 10000, true);
    }

    void ServerRegisterPartyRPC(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        Param2<string, ref array<ref string>> data;
        if (!ctx.Read(data))
            return;

        string result;
		JsonSerializer().WriteToString(data, false, result);
		Print("[SchanaParty] ServerRegisterPartyRPC " + result);
		
		ServerRegisterParty(data.param1, data.param2);
    }

    void ServerRegisterParty(string key, ref array<ref string> ids)
    {
        Print("[SchanaParty] Register " + ids.Count().ToString() + " to " + key);
		auto party_members = new ref set<string>();
		foreach (string id : ids)
		{
			party_members.Insert(id);
		}
        configurations.Set(key, party_members);
        SendInfo();
    }

    private ref map<ref string, ref set<ref string>> GetParties()
    {
        auto parties = new ref map<ref string, ref set<ref string>>();

        foreach(auto owner_id, auto party_ids : configurations)
        {
			if (party_ids && party_ids.Count() > 0)
			{
				auto validated_party_ids = new ref set<ref string>();
	
	            foreach(string member_id : party_ids)
	            {
	                if(configurations.Get(member_id) && configurations.Get(member_id).Find(owner_id) != -1)
					{
	                    validated_party_ids.Insert(member_id);
	                }
					// else if(member_id == "a" || member_id == "b" || member_id == "c")
	                // {
	                //     validated_party_ids.Insert(member_id);
	                // }
	            }
	
	            if (validated_party_ids.Count() > 0)
	            {
	                parties.Insert(owner_id, validated_party_ids);
	            }
			}
        }

        return parties;
    }

    private ref map<ref string, ref vector> GetPositions()
    {
        auto positions = new ref map<ref string, ref vector>();
		
		MissionServer mission = MissionServer.Cast(GetGame().GetMission());

        foreach(Man man : mission.m_Players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if(player && player.GetIdentity())
            {
                positions.Insert(player.GetIdentity().GetId(), player.GetPosition());
            }
        }
		
		// positions.Insert("a", "4500 350 10000" + Vector(Math.RandomFloat(-100, 100), 0, 0));
		// positions.Insert("b", "4500 100 2500" + Vector(Math.RandomFloat(-100, 100), 0, 0));
		// positions.Insert("c", "14000 200 13000" + Vector(Math.RandomFloat(-100, 100), 0, 0));

        return positions;
    }

    void SendInfo()
    {
        Print("[SchanaParty] SendInfo");
		auto id_map = new map<string, PlayerBase>();
		
		MissionServer mission = MissionServer.Cast(GetGame().GetMission());

        foreach(Man man : mission.m_Players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if(player && player.GetIdentity())
            {
                id_map.Insert(player.GetIdentity().GetId(), player);
            }
        }
		
		string result;
		auto positions = GetPositions();
		auto parties = GetParties();
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
				foreach (string party_id : party_ids)
				{
					ids.Insert(party_id);
					locations.Insert(positions.Get(party_id));
				}
				auto info = new Param2<ref array<ref string>, ref array<ref vector>>(ids, locations);
				
				JsonSerializer().WriteToString(info, false, result);
				Print("[SchanaParty] SendInfo to " + id + " " + result);
				
				GetRPCManager().SendRPC("SchanaModParty", "ClientUpdatePartyInfoRPC", info, false, id_map.Get(id).GetIdentity());
			}
		}
		
		auto all_player_ids = new ref array<ref string>;
		auto all_player_names = new ref array<ref string>;
		foreach (auto player_id, auto player_base_player : id_map)
		{
			all_player_ids.Insert(player_id);
			all_player_names.Insert(player_base_player.GetIdentity().GetName());
		}
		// all_player_ids.Insert("a");
		// all_player_ids.Insert("b");
		// all_player_ids.Insert("c");
		// all_player_names.Insert("online A");
		// all_player_names.Insert("online B");
		// all_player_names.Insert("online C");
		// for(int i=0; i<50; ++i)
		// {
		// 	all_player_ids.Insert("id" + i.ToString());
		// 	all_player_names.Insert("player " + i.ToString());
		// }
		
		auto all_player_info = new Param2<ref array<ref string>, ref array<ref string>>(all_player_ids, all_player_names);
		
		JsonSerializer().WriteToString(all_player_info, false, result);
		Print("[SchanaParty] SendPlayers " + result);
		
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
