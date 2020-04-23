class SchanaPartyManagerServer
{
    private ref map<string, ref set<string>> configurations;

    void SchanaPartyManagerServer()
    {
        Print("[SchanaParty] Server Init");
        GetRPCManager().AddRPC("SchanaModParty", "ServerAddToPartyRPC", this, SingleplayerExecutionType.Both);
        GetRPCManager().AddRPC("SchanaModParty", "ServerRemoveFromPartyRPC", this, SingleplayerExecutionType.Both);
        GetRPCManager().AddRPC("SchanaModParty", "ServerRegisterPartyRPC", this, SingleplayerExecutionType.Both);

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.SendInfo, 10000, true);
    }
	
	void ServerAddToPartyRPC(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        Param2<string, string> data;
        if (!ctx.Read(data))
            return;
		
		ServerAddToParty(data.param1, data.param2);
    }
	
	void ServerAddToParty(string key, string id)
    {
        Print("[SchanaParty] Add " + id + " to " + key);
        configurations.Get(key).Insert(id);
        SendInfo();
    }

    void ServerRemoveFromPartyRPC(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        Param2<string, string> data;
        if (!ctx.Read(data))
            return;
		
		ServerRemoveFromParty(data.param1, data.param2);
    }
	
	void ServerRemoveFromParty(string key, string id)
    {
        Print("[SchanaParty] Remove " + id + " from " + key);
        configurations.Get(key).Remove(configurations.Get(key).Find(id));
        SendInfo();
    }

    void ServerRegisterPartyRPC(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        Param2<string, ref set<string>> data;
        if (!ctx.Read(data))
            return;
		
		ServerRegisterParty(data.param1, data.param2);
    }

    void ServerRegisterParty(string key, ref set<string> ids)
    {
        Print("[SchanaParty] Register " + ids.Count().ToString() + " to " + key);
        configurations.Set(key, ids);
        SendInfo();
    }

    private map<string, set<string>> GetParties()
    {
        auto parties = new map<string, set<string>>();

        foreach(auto key, auto ids : configurations)
        {
            auto party_ids = new set<string>;

            foreach(string id : configurations.Get(key))
            {
                if(configurations.Get(id) && configurations.Get(id).Find(key) != -1)
                {
                    party_ids.Insert(id);
                }
            }

            if (ids.Count() > 0)
            {
                parties.Insert(key, party_ids);
            }
        }

        return parties;
    }

    private map<string, vector> GetPositions()
    {
        auto positions = new map<string, vector>();
		
		MissionServer mission = MissionServer.Cast(GetGame().GetMission());

        foreach(Man man : mission.m_Players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if(player && player.GetIdentity())
            {
                positions.Insert(player.GetIdentity().GetId(), player.GetPosition());
            }
        }

        return positions;
    }
	
	private ref map<ref string, ref set<ref string>> GetMockParties()
    {
        auto parties = new ref map<ref string, ref set<ref string>>();
		ref set<ref string> party_members = new ref set<ref string>();
		party_members.Insert("a");
		party_members.Insert("b");
		party_members.Insert("c");

        MissionServer mission = MissionServer.Cast(GetGame().GetMission());

        foreach(Man man : mission.m_Players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if(player && player.GetIdentity())
            {
                parties.Insert(player.GetIdentity().GetId(), party_members);
            }
        }
		
		string result;
		JsonSerializer().WriteToString(parties, false, result);
		Print("[SchanaParty] MockParties " + result);

        return parties;
    }

    private ref map<ref string, ref vector> GetMockPositions()
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
		
		positions.Insert("a", "4500 400 10000");
		positions.Insert("b", "4500 100 2500");
		positions.Insert("c", "14000 200 13000");

		string result;
		JsonSerializer().WriteToString(positions, false, result);
		Print("[SchanaParty] MockPositions " + result);
		
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
		
		auto positions = GetMockPositions();
		auto parties = GetMockParties();
		foreach (auto id, auto party_ids : parties)
		{
			auto ids = new ref array<ref string>;
			auto locations = new ref array<ref vector>;
			foreach (string party_id : party_ids)
			{
				ids.Insert(party_id);
				locations.Insert(positions.Get(party_id));
			}
			auto info = new Param2<ref array<ref string>, ref array<ref vector>>(ids, locations);
			
			string result;
			JsonSerializer().WriteToString(info, false, result);
			Print("[SchanaParty] SendInfo " + result);
			
			GetRPCManager().SendRPC("SchanaModParty", "ClientUpdatePartyInfoRPC", info, false, id_map.Get(id).GetIdentity());
		}
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
