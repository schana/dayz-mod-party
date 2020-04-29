class SchanaPartyManagerClient
{
    private ref map<ref string, ref SchanaPartyNametagsMenu> m_SchanaNametags;
    private ref SchanaPartyPositions positions;
    private ref map<ref string, ref float> healths;
	private ref map<ref string, ref string> allPlayers;
	private ref map<ref string, ref string> sortingMap;
	
    private bool hasRegistered = false;
	private bool canRegisterAgain = true;
    private bool schanaDebug = false;
	
    private bool debugToggle = false;

    void SchanaPartyManagerClient()
    {
        Print("[SchanaParty] Client Init 1.0.0");
        positions = new SchanaPartyPositions();
		healths = new ref map<ref string, ref float>();
		allPlayers = new ref map<ref string, ref string>();
		sortingMap = new ref map<ref string, ref string>();
        m_SchanaNametags = new map<ref string, ref SchanaPartyNametagsMenu>();
        GetRPCManager().AddRPC("SchanaModParty", "ClientUpdatePartyInfoRPC", this, SingleplayerExecutionType.Both);
		GetRPCManager().AddRPC("SchanaModParty", "ClientUpdatePlayersInfoRPC", this, SingleplayerExecutionType.Both);

        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.Update, 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.ResetRegisterLock, 1000, true);
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.RenewRegistration, 5000, true);

        if (schanaDebug)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.DebugUpdateMockRegistration, 10000, true);
        }
    }

    void ~SchanaPartyManagerClient()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(this.Update);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(this.ResetRegisterLock);
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(this.RenewRegistration);

        if (schanaDebug)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(this.DebugUpdateMockRegistration);
        }
    }

    void DebugUpdateMockRegistration()
    {
        Print("[SchanaParty] DebugUpdateMockRegister");

        PlayerBase activePlayer = PlayerBase.Cast(GetGame().GetPlayer());
		if (activePlayer && activePlayer.GetIdentity())
        {
			string activePlayerId = activePlayer.GetIdentity().GetId();

            auto members = GetSchanaPartySettings().GetMembers();
			auto mock_members = new ref array<ref string>;
			mock_members.Insert(activePlayerId);
            foreach (auto member : members)
            {
				Param2<ref string, ref array<ref string>> data;
                if (member != "c" || debugToggle)
                {
                    data = new Param2<ref string, ref array<ref string>>(member, mock_members);
				}
				else
				{
					data = new Param2<ref string, ref array<ref string>>(member, new ref array<ref string>);
				}
				string result;
        		JsonSerializer().WriteToString(data, false, result);
                SchanaPartyUtils.LogMessage("DebugRegistration " + result);
                GetRPCManager().SendRPC("SchanaModParty", "ServerRegisterPartyRPC", data);
            }
			debugToggle = !debugToggle;
        }
    }

    void RenewRegistration()
    {
        hasRegistered = false;
    }
	
	void ResetRegisterLock()
	{
		canRegisterAgain = true;
	}

    void ClientUpdatePartyInfoRPC(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        Param3<ref array<ref string>, ref array<ref vector>, ref array<ref float>> data;
        if (!ctx.Read(data))
            return;
		
		string result;
        JsonSerializer().WriteToString(data, false, result);
        SchanaPartyUtils.LogMessage("Update PartyInfo " + result);

        ClientUpdatePartyInfo(data.param1, data.param2, data.param3);
    }

    void ClientUpdatePartyInfo(ref array<ref string> party_ids, ref array<ref vector> server_positions, ref array<ref float> server_healths)
    {
        positions.Replace(party_ids, server_positions);
        healths.Clear();

        int i;
        for (i = 0; i < party_ids.Count(); ++i)
        {
            healths.Insert(party_ids[i], server_healths[i]);
        }
    }
	
	void ClientUpdatePlayersInfoRPC(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
    {
        Param2<ref array<ref string>, ref array<ref string>> data;
        if (!ctx.Read(data))
            return;

		string result;
        JsonSerializer().WriteToString(data, false, result);
        SchanaPartyUtils.LogMessage("Update PlayersInfo " + result);

        ClientUpdatePlayersInfo(data.param1, data.param2);
    }
	
	void ClientUpdatePlayersInfo(ref array<ref string> player_ids, ref array<ref string> player_names)
    {
        allPlayers.Clear();
		
		PlayerBase activePlayer = PlayerBase.Cast(GetGame().GetPlayer());
		if (activePlayer && activePlayer.GetIdentity())
        {
			string activePlayerId = activePlayer.GetIdentity().GetId();
	
	        int i;
	        for (i = 0; i < player_ids.Count(); ++i)
	        {
				if(player_ids[i] != activePlayerId)
				{
	            	allPlayers.Insert(player_ids[i], player_names[i]);
				}
	        }
		}
    }

    private void Update()
    {
        PlayerBase activePlayer = PlayerBase.Cast(GetGame().GetPlayer());

        if (activePlayer && activePlayer.GetIdentity())
        {
            string activePlayerId = activePlayer.GetIdentity().GetId();

            UpdateRegistration(activePlayerId);
			AddAndUpdateNametags();
			RemoveInvalidNametags();
			UpdateNametagsWithLocalPlayers(activePlayerId);
			
			UpdateMenuListSorting();
        }
    }
	
	private void UpdateRegistration(string activePlayerId)
	{
		if (!hasRegistered && canRegisterAgain)
        {
			Print("[SchanaParty] Send Updated Registration");
            auto members = GetSchanaPartySettings().GetMembers();
            auto data = new Param2<ref string, ref array<ref string>>(activePlayerId, members);
            GetRPCManager().SendRPC("SchanaModParty", "ServerRegisterPartyRPC", data);
            hasRegistered = true;
			canRegisterAgain = false;
        }
	}
	
	private void AddAndUpdateNametags()
	{
		auto current_positions = positions.Get();
		
		string result;
       	JsonSerializer().WriteToString(current_positions, false, result);
       	SchanaPartyUtils.LogMessage("Positions " + result);
        foreach (string party_id, vector position : current_positions)
        {
            if (!m_SchanaNametags.Get(party_id))
            {
				Print("[SchanaParty] Adding nametag " + party_id);
                m_SchanaNametags[party_id] = new SchanaPartyNametagsMenu(null);
            }
            m_SchanaNametags[party_id].SchanaPartyUpdatePosition(position);
            m_SchanaNametags[party_id].SchanaPartyUpdateHealth(healths.Get(party_id));
            m_SchanaNametags[party_id].SchanaPartyUpdateName(GetSchanaPartySettings().GetName(party_id));
        }
	}
	
	private void RemoveInvalidNametags()
	{
		auto current_positions = positions.Get();
		
		auto member_ids = m_SchanaNametags.GetKeyArray();
		foreach (auto member_id : member_ids)
		{
			if(!current_positions.Get(member_id))
			{
				Print("[SchanaParty] Removing nametag " + member_id);
				m_SchanaNametags[member_id].SchanaPartySetRemoveFlag();
				m_SchanaNametags.Remove(member_id);
			}
		}
	}
	
	private void UpdateNametagsWithLocalPlayers(string activePlayerId)
	{
		auto current_positions = positions.Get();
		
		foreach (Man man : ClientData.m_PlayerBaseList)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if (player && player.GetIdentity())
            {
                string id = player.GetIdentity().GetId();
                if (current_positions.Get(id) && id != activePlayerId)
                {
                    m_SchanaNametags[id].SchanaPartyUpdatePlayer(player);
                }
            }
        }
	}
	
	private void UpdateMenuListSorting()
	{
		sortingMap.Clear();
		auto member_ids = m_SchanaNametags.GetKeyArray();
		foreach (auto sorting_id : member_ids)
		{
			sortingMap.Insert(GetSchanaPartySettings().GetName(sorting_id) + sorting_id, sorting_id);
		}
		auto sorted_keys = sortingMap.GetKeyArray();
		sorted_keys.Sort();
		int sortedIndex = 0;
		foreach (auto sorted_key : sorted_keys)
		{
			m_SchanaNametags[sortingMap.Get(sorted_key)].SchanaPartyUpdateListIndex(sortedIndex);
			++sortedIndex;
		}
	}

    void AddPlayerToParty(string id)
    {
		Print("[SchanaParty] AddPlayerToParty " + id);
		GetSchanaPartySettings().Add(id, GetOnlinePlayers().Get(id));
		hasRegistered = false;
    }

    void RemovePlayerFromParty(string id)
    {
		Print("[SchanaParty] RemovePlayerFromParty " + id);
        GetSchanaPartySettings().Remove(id);
		positions.Get().Remove(id);
		hasRegistered = false;
    }
	
	bool IsPartyMemberOnline(string id)
	{
		return positions.Get().Contains(id);
	}
	
	bool IsPartyMemberOnlineButNotMutual(string id)
	{
		return !positions.Get().Contains(id) && allPlayers.Contains(id);
	}
	
	ref map<ref string, ref string> GetOnlinePlayers()
	{
		return allPlayers;
	}

    ref map<ref string, ref vector> GetPositions()
    {
		return positions.Get();
    }
}
