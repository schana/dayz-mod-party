class SchanaPartyManagerServer
{
    private ref map<string, ref set<string>> configurations;

    void SchanaPartyManagerServer()
    {
        Print("[SchanaParty] Server Init");
		configurations = new ref map<string, ref set<string>>();
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

        foreach(auto key, auto ids : configurations)
        {
            auto party_ids = new ref set<ref string>();

            foreach(string id : configurations.Get(key))
            {
                if(id == "a" || id == "b" || id == "c" || configurations.Get(id) && configurations.Get(id).Find(key) != -1)
                {
                    party_ids.Insert(id);
                }
            }

            if (party_ids.Count() > 0)
            {
                parties.Insert(key, party_ids);
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
		
		positions.Insert("a", "4500 400 10000" + Vector(Math.RandomFloat(-100, 100), 0, 0));
		positions.Insert("b", "4500 100 2500" + Vector(Math.RandomFloat(-100, 100), 0, 0));
		positions.Insert("c", "14000 200 13000" + Vector(Math.RandomFloat(-100, 100), 0, 0));

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
				
				string result;
				JsonSerializer().WriteToString(info, false, result);
				Print("[SchanaParty] SendInfo to " + id + " " + result);
				
				GetRPCManager().SendRPC("SchanaModParty", "ClientUpdatePartyInfoRPC", info, false, id_map.Get(id).GetIdentity());
			}
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
