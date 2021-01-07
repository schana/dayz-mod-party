class SchanaPartyMarkerManagerServer {
    protected ref map<string, ref array<ref SchanaPartyMarkerInfo>> markers;
    protected bool canSendInfo = true;

    void SchanaPartyMarkerManagerServer () {
        SchanaPartyUtils.LogMessage ("PartyMarker Server Init");
        markers = new map<string, ref array<ref SchanaPartyMarkerInfo>> ();
        GetRPCManager ().AddRPC ("SchanaModParty", "ServerRegisterMarkersRPC", this, SingleplayerExecutionType.Both);

        GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.SendMarkers, GetSchanaPartyServerSettings ().GetSendMarkerFrequency () * 1000, true);
    }

    void ~SchanaPartyMarkerManagerServer () {
        GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).Remove (this.SendMarkers);
    }

    protected void ResetSendInfoLock () {
        canSendInfo = true;
    }

    void ServerRegisterMarkersRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		SchanaPartyUtils.Trace ("ServerRegisterMarkersRPC Start");
        Param1<ref array<SchanaPartyMarkerInfo>> data;
        if (!ctx.Read (data))
            return;
        if (SchanaPartyUtils.WillLog (SchanaPartyUtils.DEBUG)) {
            string result;
            JsonSerializer ().WriteToString (data, false, result);
            SchanaPartyUtils.Debug ("ServerRegisterMarkersRPC " + result);
        }
		if (!data.param1){
			return;
		}
		ref array<SchanaPartyMarkerInfo> playerMarkers = new array<SchanaPartyMarkerInfo>;
		playerMarkers.Copy(data.param1);
		ref array<ref SchanaPartyMarkerInfo> playerMarkersrefs =  new array<ref SchanaPartyMarkerInfo>;
		for (int i = 0; i < playerMarkers.Count(); ++i){
			playerMarkersrefs.Insert(playerMarkers.Get(i));
		}
        ServerRegisterMarkers (sender.GetId (), playerMarkersrefs);
    }

    void ServerRegisterMarkers (string id, ref array<ref SchanaPartyMarkerInfo> playerMarkers) {
		if (!playerMarkers){
			return;
		}
		
		SchanaPartyUtils.Trace ("ServerRegisterMarkers Start");
		
        int maxMarkers = GetSchanaPartyServerSettings ().GetMaxMarkers ();
		SchanaPartyUtils.Trace ("ServerRegisterMarkers maxMarkers: " + maxMarkers);
		
		int MAXXLOOPCOUNT = 50;
		if (SchanaPartyUtils.WillLog (SchanaPartyUtils.TRACE)) {
			playerMarkers.Debug();
		}
        if (maxMarkers > 0 && playerMarkers.Count () > maxMarkers && playerMarkers) {
            while (playerMarkers.Count () > maxMarkers && MAXXLOOPCOUNT > 0) {
                playerMarkers.RemoveOrdered (0);
				MAXXLOOPCOUNT--;
            }
        }
		
		if (SchanaPartyUtils.WillLog (SchanaPartyUtils.TRACE)) {
			playerMarkers.Debug();
		}
		
		SchanaPartyUtils.Trace ("ServerRegisterMarkers markers Set");
		
        markers.Set (id, playerMarkers);
    }

	void SendMarkers(){
		SchanaPartyUtils.Trace ("SendMarkers Start");
        if (canSendInfo) {
            canSendInfo = false;
			GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.ResetSendInfoLock, 1000, false);
			thread SendMarkersThread ();
		}
		
	}

    void SendMarkersThread () {
			SchanaPartyUtils.Trace ("SendMarkersThread Start");
            auto id_map = new ref map<string, DayZPlayer> ();
            array<Man> players = new array<Man>;
            GetGame ().GetPlayers (players);
			int i = 0;
            for (i = 0; i < players.Count (); ++i ) {
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
            for ( i = 0; i <  parties.Count (); ++i) {
				SchanaPartyUtils.Trace ("SendMarkers parties " + i);
				DayZPlayer plr = DayZPlayer.Cast (id_map.Get ( parties.GetKey (i) ));
				if (plr && plr.GetIdentity () && plr.IsAlive () && parties.GetElement (i)) {
					SchanaPartyUtils.Trace ("SendMarkers Begin " + parties.GetKey (i));
					if ( parties.GetElement (i).Count () > 0 ){
						SendMarkerInfoToPlayer (parties.GetKey (i), parties.GetElement (i), plr.GetIdentity ());
						SchanaPartyUtils.Trace ("SendMarkers End " + parties.GetKey (i));
					}
				}
            }

			SchanaPartyUtils.Trace ("SendMarkersThread Finish");
    }
	
    protected void SendMarkerInfoToPlayer (string id, ref set<string> party_ids, PlayerIdentity player) {
		if (!player || !markers){
			return;
		}
        SchanaPartyUtils.Trace ("SendMarkerInfoToPlayer Start");
        auto playerMarkers = new ref array<ref SchanaPartyMarkerInfo>;
        foreach (string party_id : party_ids) {
            if (markers.Contains (party_id) && markers.Get (party_id)) {
                for (int i = 0; i < markers.Get (party_id).Count(); ++i) {
					if (markers.Get (party_id).Get (i)){
						playerMarkers.Insert (markers.Get (party_id).Get (i));
					}
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
        SchanaPartyUtils.Trace ("SendMarkerInfoToPlayer Finish");
    }
}

static ref SchanaPartyMarkerManagerServer g_SchanaPartyMarkerManagerServer;
static ref SchanaPartyMarkerManagerServer GetSchanaPartyMarkerManagerServer () {
    if (g_Game.IsServer () && !g_SchanaPartyMarkerManagerServer) {
        g_SchanaPartyMarkerManagerServer = new SchanaPartyMarkerManagerServer;
    }
    return g_SchanaPartyMarkerManagerServer;
}