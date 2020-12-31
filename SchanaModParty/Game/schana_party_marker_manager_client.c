class SchanaPartyMarkerManagerClient {
    private ref array<ref SchanaPartyMarkerInfo> markers;
    private ref array<ref SchanaPartyMarkerInfo> serverMarkers;
    private ref array<ref SchanaPartyMarkerMenu> markerMenus;
    private bool initialized = false;
    private bool canSend = true;

    void SchanaPartyMarkerManagerClient () {
        SchanaPartyUtils.LogMessage ("PartyMarker Client Init");
        markers = new ref array<ref SchanaPartyMarkerInfo>;
        serverMarkers = new ref array<ref SchanaPartyMarkerInfo>;
        markerMenus = new ref array<ref SchanaPartyMarkerMenu>;

        GetRPCManager ().AddRPC ("SchanaModParty", "ClientUpdatePartyMarkersRPC", this, SingleplayerExecutionType.Both);
        GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.ResetSendLock, 1000, true);
    }

    void ~SchanaPartyMarkerManagerClient () {
        GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).Remove (this.ResetSendLock);
    }

    private void ResetSendLock () {
        canSend = true;
    }

    bool IsInitialized () {
        return initialized;
    }

    void Init () {
        auto positions = GetSchanaPartyMarkerSettings ().GetMarkers ();
        for (int i = 0; i < positions.Count (); ++i) {
            markers.Insert (new SchanaPartyMarkerInfo (GetNextName (), positions[i]));
        }
        ClientUpdatePartyMarkers (serverMarkers);
        initialized = true;
    }

    void ClientUpdatePartyMarkersRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        Param1<ref array<ref SchanaPartyMarkerInfo>> data;
        if (!ctx.Read (data))
            return;

        ClientUpdatePartyMarkers (data.param1);
    }

    void ClientUpdatePartyMarkers (ref array<ref SchanaPartyMarkerInfo> newServerMarkers) {
        SchanaPartyUtils.LogMessage ("ClientUpdatePartyMarkers");
        serverMarkers = newServerMarkers;
        int i;
        for (i = 0; i < markerMenus.Count (); ++i) {
            if (markerMenus.Get (i)) {
                markerMenus.Get (i).SchanaPartySetRemoveFlag ();
            }
        }
        for (i = 0; i < serverMarkers.Count (); ++i) {
            markerMenus.Insert (new SchanaPartyMarkerMenu (serverMarkers.Get (i).GetName (), serverMarkers.Get (i).GetPosition ()));
        }
        for (i = 0; i < markers.Count (); ++i) {
            markerMenus.Insert (new SchanaPartyMarkerMenu (markers.Get (i).GetName (), markers.Get (i).GetPosition ()));
        }
    }

    void Add (SchanaPartyMarkerInfo marker) {
        GetSchanaPartyMarkerSettings ().Add (marker.GetPosition ());
        markers.Insert (marker);
        ClientUpdatePartyMarkers (serverMarkers);
        Send ();
    }

    void Reset () {
        GetSchanaPartyMarkerSettings ().Clear ();
        markers.Clear ();
        ClientUpdatePartyMarkers (serverMarkers);
        Send ();
    }

    void Send (bool tryAgain = true) {
        if (canSend) {
            SchanaPartyUtils.LogMessage ("SendMarkers");
            auto data = new Param1<ref array<ref SchanaPartyMarkerInfo>> (markers);
            GetRPCManager ().SendRPC ("SchanaModParty", "ServerRegisterMarkersRPC", data);

            canSend = false;
        } else if (tryAgain) {
            GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.Send, 1200, false, false);
        }
    }

    string GetNextName () {
		if (GetGame ().GetPlayer () && GetGame ().GetPlayer ().GetIdentity ()){
        return GetGame ().GetPlayer ().GetIdentity ().GetName () + " " + (markers.Count () + 1).ToString ();
		}
		return "[NULL] " + (markers.Count () + 1).ToString ();
    }
}

static ref SchanaPartyMarkerManagerClient g_SchanaPartyMarkerManagerClient;
static ref SchanaPartyMarkerManagerClient GetSchanaPartyMarkerManagerClient () {
    if (g_Game.IsClient () && !g_SchanaPartyMarkerManagerClient) {
        g_SchanaPartyMarkerManagerClient = new SchanaPartyMarkerManagerClient;
    }
    return g_SchanaPartyMarkerManagerClient;
}