class SchanaPartyMarkerManagerClient {
    private ref array<ref SchanaPartyMarkerInfo> markers;
    private ref array<ref SchanaPartyMarkerInfo> serverMarkers;
    private ref array<ref SchanaPartyMarkerMenu> markerMenus;
    private bool initialized = false;
    private bool canSend = true;

    void SchanaPartyMarkerManagerClient () {
        SchanaPartyUtils.LogMessage ("PartyMarker Client Init");
        markers = new ref array<ref SchanaPartyMarkerInfo>;
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
        initialized = true;
    }

    void ClientUpdatePartyMarkersRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        Param1<ref array<ref SchanaPartyMarkerInfo>> data;
        if (!ctx.Read (data))
            return;

        ClientUpdatePartyMarkers (data.param1);
    }

    void ClientUpdatePartyMarkers (ref array<ref SchanaPartyMarkerInfo> serverMarkers) {
        SchanaPartyUtils.LogMessage ("ClientUpdatePartyMarkers");
        markerMenus.Clear ();
        for (int i = 0; i < serverMarkers.Count (); ++i) {
            markerMenus.Insert (new SchanaPartyMarkerMenu (serverMarkers.Get (i).GetName (), serverMarkers.Get (i).GetPosition ()));
        }
    }

    void Add (SchanaPartyMarkerInfo marker) {
        GetSchanaPartyMarkerSettings ().Add (marker.GetPosition ());
        markers.Insert (marker);
        Send ();
    }

    void Reset () {
        GetSchanaPartyMarkerSettings ().Clear ();
        markers.Clear ();
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