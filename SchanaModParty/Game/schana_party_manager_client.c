class SchanaPartyManagerClient {
    protected ref map<string, ref SchanaPartyNametagsMenu> m_SchanaNametags;
    protected ref SchanaPartyPositions positions;
    protected ref map<string, float> healths;
    protected ref map<string, string> allPlayers;
    protected ref map<string, string> sortingMap;

    void SchanaPartyManagerClient () {
        SchanaPartyUtils.LogMessage ("Client Init " + SCHANA_PARTY_VERSION);

        positions = new ref SchanaPartyPositions ();
        healths = new ref map<string, float> ();
        allPlayers = new ref map<string, string> ();
        sortingMap = new ref map<string, string> ();
        m_SchanaNametags = new map<string, ref SchanaPartyNametagsMenu> ();

        GetRPCManager ().AddRPC ("SchanaModParty", "ClientUpdatePartyInfoRPC", this, SingleplayerExecutionType.Both);
        GetRPCManager ().AddRPC ("SchanaModParty", "ClientUpdatePlayersInfoRPC", this, SingleplayerExecutionType.Both);

        GetGame ().GetCallQueue (CALL_CATEGORY_GUI).CallLater (this.Update, 2000, true);
        GetGame ().GetCallQueue (CALL_CATEGORY_GUI).CallLater (this.RenewRegistration, 15000, true);
    }

    void ~SchanaPartyManagerClient () {
        GetGame ().GetCallQueue (CALL_CATEGORY_GUI).Remove (this.Update);
        GetGame ().GetCallQueue (CALL_CATEGORY_GUI).Remove (this.RenewRegistration);
    }

    void RenewRegistration () {
        DayZPlayer activePlayer = DayZPlayer.Cast (GetGame ().GetPlayer ());

        if (activePlayer && activePlayer.GetIdentity () && activePlayer.IsAlive ()) {
            string activePlayerId = activePlayer.GetIdentity ().GetId ();

            UpdateRegistration (activePlayerId);
        }
    }

    void ClientUpdatePartyInfoRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        SchanaPartyUtils.LogMessage ("ClientUpdatePartyInfoRPC");
        Param3<ref array<string>, ref array<vector>, ref array<float>> data;
        if (!ctx.Read (data))
            return;
		ref array<string> party_ids = new array<string>;
		ref array<vector> server_positions = new array<vector>;
		ref array<float> server_healths = new array<float>;
		party_ids.Copy(data.param1);
		server_positions.Copy(data.param2);
		server_healths.Copy(data.param3);
        ClientUpdatePartyInfo (party_ids, server_positions, server_healths);
    }

    void ClientUpdatePartyInfo (ref array<string> party_ids, ref array<vector> server_positions, ref array<float> server_healths) {
        positions.Replace (party_ids, server_positions);
        healths.Clear ();
		/*
        party_ids.Debug ();
        server_positions.Debug ();
        server_healths.Debug ();
		*/
        int i;
        for (i = 0; i < party_ids.Count (); ++i) {
            healths.Insert (party_ids.Get (i), server_healths.Get (i));
        }

    }

    void ClientUpdatePlayersInfoRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        SchanaPartyUtils.LogMessage ("ClientUpdatePlayersInfoRPC");
        Param2<ref array<string>, ref array<string>> data;
        if (!ctx.Read (data))
            return;

		ref array<string> player_ids = new array<string>;
		ref array<string> player_names = new array<string>;
		player_ids.Copy(data.param1);
		player_names.Copy(data.param2);
        ClientUpdatePlayersInfo (player_ids, player_names);
    }

    void ClientUpdatePlayersInfo (ref array<string> player_ids, ref array<string> player_names) {
        allPlayers.Clear ();

        DayZPlayer activePlayer = DayZPlayer.Cast (GetGame ().GetPlayer ());
        if (activePlayer && activePlayer.GetIdentity () && activePlayer.IsAlive ()) {
            string activePlayerId = activePlayer.GetIdentity ().GetId ();

            int i;
            for (i = 0; i < player_ids.Count (); ++i) {
                if (player_ids.Get (i) != activePlayerId) {
                    allPlayers.Insert (player_ids.Get (i), player_names.Get (i));
                }
            }
        }
    }

    protected void Update () {
        DayZPlayer activePlayer = DayZPlayer.Cast (GetGame ().GetPlayer ());

        if (!m_SchanaNametags) {
            m_SchanaNametags = new map<string, ref SchanaPartyNametagsMenu> ();
        }

        if (activePlayer && activePlayer.GetIdentity () && activePlayer.IsAlive ()) {
            string activePlayerId = activePlayer.GetIdentity ().GetId ();

            AddAndUpdateNametags ();
            RemoveInvalidNametags ();
            UpdateNametagsWithLocalPlayers (activePlayerId);

            UpdateMenuListSorting ();
        }
    }

    protected void UpdateRegistration (string activePlayerId) {
        auto members = GetSchanaPartySettings ().GetMembers ();
        auto data = new Param2<string, ref array<string>> (activePlayerId, members);
        GetRPCManager ().SendRPC ("SchanaModParty", "ServerRegisterPartyRPC", data);
    }

    protected void AddAndUpdateNametags () {
        auto current_positions = positions.Get ();
        foreach (string party_id, vector position : current_positions) {
            if (!m_SchanaNametags.Contains (party_id)) {
                SchanaPartyUtils.LogMessage ("Adding nametag " + party_id);
                m_SchanaNametags.Insert (party_id, new SchanaPartyNametagsMenu (null));
            }
            if (!m_SchanaNametags.Get (party_id)) {
                m_SchanaNametags.Set (party_id, new SchanaPartyNametagsMenu (null));
            }
            m_SchanaNametags.Get (party_id).SchanaPartyUpdatePosition (position);
            m_SchanaNametags.Get (party_id).SchanaPartyUpdateHealth (healths.Get (party_id));
            m_SchanaNametags.Get (party_id).SchanaPartyUpdateName (GetSchanaPartySettings ().GetName (party_id));
        }
    }

    protected void RemoveInvalidNametags () {
        auto current_positions = positions.Get ();

        auto member_ids = m_SchanaNametags.GetKeyArray ();
        foreach (auto member_id : member_ids) {
            if (!current_positions.Contains (member_id)) {
                SchanaPartyUtils.LogMessage ("Removing nametag " + member_id);
                m_SchanaNametags.Get (member_id).SchanaPartySetRemoveFlag ();
                m_SchanaNametags.Remove (member_id);
            }
        }
    }

    protected void UpdateNametagsWithLocalPlayers (string activePlayerId) {
        auto current_positions = positions.Get ();

        foreach (Man man : ClientData.m_PlayerBaseList) {
            DayZPlayer player = DayZPlayer.Cast (man);
            if (player && player.GetIdentity () && player.IsAlive ()) {
                string id = player.GetIdentity ().GetId ();
                if (current_positions.Contains (id) && id != activePlayerId) {
                    if (!m_SchanaNametags.Contains (id)) {
                        SchanaPartyUtils.LogMessage ("Adding nametag " + id);
                        m_SchanaNametags.Insert (id, new SchanaPartyNametagsMenu (null));
                    }
                    m_SchanaNametags.Get (id).SchanaPartyUpdatePlayer (player);
                }
            }
        }
    }

    protected void UpdateMenuListSorting () {
        sortingMap.Clear ();
        auto member_ids = m_SchanaNametags.GetKeyArray ();
        foreach (auto sorting_id : member_ids) {
            string name = GetSchanaPartySettings ().GetName (sorting_id);
            name.ToLower ();
            sortingMap.Insert (name + sorting_id, sorting_id);
        }
        auto sorted_keys = sortingMap.GetKeyArray ();
        sorted_keys.Sort ();
        int sortedIndex = 0;
        foreach (auto sorted_key : sorted_keys) {
            m_SchanaNametags.Get (sortingMap.Get (sorted_key)).SchanaPartyUpdateListIndex (sortedIndex);
            ++sortedIndex;
        }
    }

    void AddPlayerToParty (string id) {
        SchanaPartyUtils.LogMessage ("AddPlayerToParty " + id);
        GetSchanaPartySettings ().Add (id, GetOnlinePlayers ().Get (id));
        RenewRegistration ();
    }

    void RemovePlayerFromParty (string id) {
        SchanaPartyUtils.LogMessage ("RemovePlayerFromParty " + id);
        GetSchanaPartySettings ().Remove (id);
        positions.Get ().Remove (id);
        RenewRegistration ();
    }

    bool IsPartyMemberOnline (string id) {
        return positions.Get ().Contains (id);
    }

    bool IsPartyMemberOnlineButNotMutual (string id) {
        return !positions.Get ().Contains (id) && allPlayers.Contains (id);
    }

    ref map<string, string> GetOnlinePlayers () {
        return allPlayers;
    }

    ref map<string, vector> GetPositions () {
        return positions.Get ();
    }
}