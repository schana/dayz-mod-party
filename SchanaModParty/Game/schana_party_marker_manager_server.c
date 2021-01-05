class SchanaPartyMarkerManagerServer {
    protected ref map<string, ref array<ref SchanaPartyMarkerInfo>> markers;
    protected bool canSendInfo = true;

    void SchanaPartyMarkerManagerServer () {
        SchanaPartyUtils.LogMessage ("PartyMarker Server Init");
        markers = new ref map<string, ref array<ref SchanaPartyMarkerInfo>> ();
        GetRPCManager ().AddRPC ("SchanaModParty", "ServerRegisterMarkersRPC", this, SingleplayerExecutionType.Both);

        GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.SendMarkers, 10000, true);
        GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.ResetSendInfoLock, GetSchanaPartyServerSettings ().GetSendMarkerFrequency () * 1000, true);
    }

    void ~SchanaPartyMarkerManagerServer () {
        GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).Remove (this.SendMarkers);
        GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).Remove (this.ResetSendInfoLock);
    }

    protected void ResetSendInfoLock () {
        canSendInfo = true;
    }

    void ServerRegisterMarkersRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        Param1<ref array<ref SchanaPartyMarkerInfo>> data;
        if (!ctx.Read (data))
            return;

        if (SchanaPartyUtils.WillLog (SchanaPartyUtils.DEBUG)) {
            string result;
            JsonSerializer ().WriteToString (data, false, result);
            SchanaPartyUtils.Debug ("ServerRegisterMarkersRPC " + result);
        }

        thread ServerRegisterMarkers (sender.GetId (), data.param1);
    }

    void ServerRegisterMarkers (string id, ref array<ref SchanaPartyMarkerInfo> playerMarkers) {
        int maxMarkers = GetSchanaPartyServerSettings ().GetMaxMarkers ();
        if (maxMarkers != -1) {
            while (playerMarkers.Count () > maxMarkers) {
                playerMarkers.RemoveOrdered (0);
            }
        }
        markers.Set (id, playerMarkers);

        SendMarkers ();
    }

    void SendMarkers () {
        if (canSendInfo) {
			SchanaPartyUtils.Trace ("SendMarkers Start");
            auto id_map = new ref map<string, DayZPlayer> ();
            array<Man> players = new array<Man>;
            GetGame ().GetPlayers (players);
			
            for (int i = 0; i < players.Count (); ++i ) {
                DayZPlayer player = DayZPlayer.Cast (players.Get (i));
                if (player && player.GetIdentity () && player.IsAlive ()) {
                    id_map.Insert (player.GetIdentity ().GetId (), player);
                }
            }

            auto manager = GetSchanaPartyManagerServer ();
			if (!manager)
				return;
            auto parties = manager.GetParties ();
			if (!parties)
				return;
            foreach (auto id, auto party_ids : parties) {
				DayZPlayer plr = DayZPlayer.Cast (id_map.Get (id));
				if (plr && plr.GetIdentity () && plr.IsAlive ()) {
					SchanaPartyUtils.Trace ("SendMarkers Begin " + id);
					SendMarkerInfoToPlayer (id, party_ids, plr.GetIdentity ());
					SchanaPartyUtils.Trace ("SendMarkers End " + id);
				}
            }

            canSendInfo = false;
        }
    }

    protected void SendMarkerInfoToPlayer (string id, ref set<string> party_ids, PlayerIdentity player) {
		if (!player){
			return;
		}
        SchanaPartyUtils.Trace ("SendMarkerInfoToPlayer Start");
        auto playerMarkers = new ref array<ref SchanaPartyMarkerInfo>;
        foreach (string party_id : party_ids) {
            if (markers.Contains (party_id)) {
                foreach (auto m : markers.Get (party_id)) {
                    playerMarkers.Insert (m);
                }
            }
        }
        auto info = new ref Param1<ref array<ref SchanaPartyMarkerInfo>> (playerMarkers);

        if (SchanaPartyUtils.WillLog (SchanaPartyUtils.DEBUG)) {
            string result;
            JsonSerializer ().WriteToString (info, false, result);
            SchanaPartyUtils.Debug ("SendMarkers to " + id + " " + result);
        }

        if (player) {
            GetRPCManager ().SendRPC ("SchanaModParty", "ClientUpdatePartyMarkersRPC", info, false, player);
        } else {
            SchanaPartyUtils.Warn ("SendMarkers failed to " + id);
        }
    }
}

static ref SchanaPartyMarkerManagerServer g_SchanaPartyMarkerManagerServer;
static ref SchanaPartyMarkerManagerServer GetSchanaPartyMarkerManagerServer () {
    if (g_Game.IsServer () && !g_SchanaPartyMarkerManagerServer) {
        g_SchanaPartyMarkerManagerServer = new SchanaPartyMarkerManagerServer;
    }
    return g_SchanaPartyMarkerManagerServer;
}