class SchanaPartyMarkerManagerClient {
    private ref array<ref SchanaPartyMarkerInfo> markers;
    private ref array<ref SchanaPartyMarkerInfo> serverMarkers;
    private ref array<ref SchanaPartyMarkerMenu> markerMenus;

    void SchanaPartyMarkerManagerClient () {
        SchanaPartyUtils.LogMessage ("PartyMarker Client Init");
        markers = new ref array<ref SchanaPartyMarkerInfo>;
        serverMarkers = new ref array<ref SchanaPartyMarkerInfo>;
        markerMenus = new ref array<ref SchanaPartyMarkerMenu>;

        GetRPCManager ().AddRPC ("SchanaModParty", "ClientUpdatePartyMarkersRPC", this, SingleplayerExecutionType.Both);
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
            if (markerMenus[i]) {
                markerMenus[i].SchanaPartySetRemoveFlag ();
            }
        }
        for (i = 0; i < serverMarkers.Count (); ++i) {
            markerMenus.Insert (new SchanaPartyMarkerMenu (serverMarkers[i].GetName (), serverMarkers[i].GetPosition ()));
        }
        for (i = 0; i < markers.Count (); ++i) {
            markerMenus.Insert (new SchanaPartyMarkerMenu (markers[i].GetName (), markers[i].GetPosition ()));
        }
    }

    void Add (SchanaPartyMarkerInfo marker) {
        markers.Insert (marker);
        Send ();
    }

    void Reset () {
        markers.Clear ();
        Send ();
    }

    void Send () {
        SchanaPartyUtils.LogMessage ("SendMarkers");
        auto data = new Param1<ref array<ref SchanaPartyMarkerInfo>> (markers);
        GetRPCManager ().SendRPC ("SchanaModParty", "ServerRegisterMarkersRPC", data);
    }

    string GetNextName () {
        return GetGame ().GetPlayer ().GetIdentity ().GetName () + " " + (markers.Count () + 1).ToString ();
    }
}

static ref SchanaPartyMarkerManagerClient g_SchanaPartyMarkerManagerClient;
static ref SchanaPartyMarkerManagerClient GetSchanaPartyMarkerManagerClient () {
    if (g_Game.IsClient () && !g_SchanaPartyMarkerManagerClient) {
        g_SchanaPartyMarkerManagerClient = new SchanaPartyMarkerManagerClient;
    }
    return g_SchanaPartyMarkerManagerClient;
}