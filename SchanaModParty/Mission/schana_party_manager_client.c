class SchanaPartyManagerClient
{
    private ref map<string, ref SchanaPartyNametagsMenu> m_SchanaNametags;
    private ref SchanaModPartySettings settings;
	private ref map<string, vector> positions;

    void SchanaPartyManagerClient()
    {
        Print("[SchanaParty] Client Init");
        settings = SchanaModPartySettings.Get();
        positions = new ref map<string, vector>();
        m_SchanaNametags = new map<string, ref SchanaPartyNametagsMenu>();
        GetRPCManager().AddRPC("SchanaModParty", "ClientUpdatePartyInfoRPC", this, SingleplayerExecutionType.Both);

        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.Update, 1000, true);
    }

    void ~SchanaPartyManagerClient()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(this.Update);
    }

    void ClientUpdatePartyInfoRPC(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        Param2<ref array<ref string>, ref array<ref vector>> data;
        if (!ctx.Read(data))
            return;
		
		string result;
		JsonSerializer().WriteToString(data, false, result);
		Print("[SchanaParty] " + result);
		
        ClientUpdatePartyInfo(data.param1, data.param2);
    }
	
	void ClientUpdatePartyInfo(ref array<ref string> party_ids, ref array<ref vector> server_positions)
	{
        Print("[SchanaParty] ClientUpdatePartyInfo");
		
		positions.Clear();
		
		int i;
		for(i=0; i<party_ids.Count(); ++i)
		{
			positions.Insert(party_ids[i], server_positions[i]);
		}
	}

    private void Update()
    {
        Print("[SchanaParty] Update");
        PlayerBase activePlayer = PlayerBase.Cast(GetGame().GetPlayer());

        if (activePlayer && activePlayer.GetIdentity())
        {
            string activePlayerId = activePlayer.GetIdentity().GetId();

            foreach (string party_id, vector position : positions)
            {
                if (!m_SchanaNametags.Get(party_id))
                {
                    m_SchanaNametags[party_id] = new SchanaPartyNametagsMenu(null);
                }
                m_SchanaNametags[party_id].SchanaPartyUpdatePosition(position);
            }
            
            foreach (Man man : ClientData.m_PlayerBaseList)
            {
                PlayerBase player = PlayerBase.Cast(man);
                if (player && player.GetIdentity())
                {
                    string id = player.GetIdentity().GetId();
                    if (positions.Get(id) && id != activePlayerId)
                    {
                        m_SchanaNametags[id].SchanaPartyUpdatePlayer(player);	
                    }
                }
            }
        }
    }
}

static ref SchanaPartyManagerClient g_SchanaPartyManagerClient;
static ref SchanaPartyManagerClient GetSchanaPartyManagerClient()
{
    if (!g_Game.IsServer() && !g_SchanaPartyManagerClient)
    {
        g_SchanaPartyManagerClient = new SchanaPartyManagerClient;
    }
    return g_SchanaPartyManagerClient;
}
