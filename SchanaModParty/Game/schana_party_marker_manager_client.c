class SchanaPartyMarkerManagerClient {
    protected ref array<ref SchanaPartyMarkerInfo> markers;
    protected ref array<ref SchanaPartyMarkerInfo> serverMarkers;
    protected ref array<ref SchanaPartyMarkerMenu> markerMenus;
    protected bool initialized = false;
    protected bool canSend = true;
	protected int CurrentCount = 1;
	
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

    protected void ResetSendLock () {
        canSend = true;
    }

    bool IsInitialized () {
        return initialized;
    }

    void Init () {
        auto positions = GetSchanaPartyMarkerSettings ().GetMarkers ();
        for (int i = 0; i < positions.Count (); ++i) {
            markers.Insert (new SchanaPartyMarkerInfo (GetNextName (), positions.Get (i)));
        }
        ClientUpdatePartyMarkers (serverMarkers);
        initialized = true;
    }

    void ClientUpdatePartyMarkersRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        Param1<array<SchanaPartyMarkerInfo>> data;
        if (!ctx.Read (data))
            return;
		array<SchanaPartyMarkerInfo> newServerMarkers = new array<SchanaPartyMarkerInfo>;
		ref array<ref SchanaPartyMarkerInfo> newServerMarkers2 = new array<ref SchanaPartyMarkerInfo>;
		newServerMarkers.Copy(data.param1);
		for (int i = 0; i < newServerMarkers.Count (); ++i){
			if (newServerMarkers.Get (i)){
				newServerMarkers2.Insert(newServerMarkers.Get (i));
			}
		}
        ClientUpdatePartyMarkers (newServerMarkers2);
    }

    void ClientUpdatePartyMarkers (ref array<ref SchanaPartyMarkerInfo> newServerMarkers) {
        SchanaPartyUtils.LogMessage ("ClientUpdatePartyMarkers");
		if (!newServerMarkers){
			return;
		}
        serverMarkers = newServerMarkers;
        int i;
        for (i = 0; i < markerMenus.Count (); ++i) {
            if (markerMenus.Get (i)) {
                markerMenus.Get (i).SchanaPartySetRemoveFlag ();
            }
        }
        for (i = 0; i < serverMarkers.Count (); ++i) {
			if (serverMarkers.Get (i)){
				markerMenus.Insert (new SchanaPartyMarkerMenu (serverMarkers.Get (i).GetName (), serverMarkers.Get (i).GetPosition ()));
			}
        }
        for (i = 0; i < markers.Count (); ++i) {
			if (markers.Get (i)){
				markerMenus.Insert (new SchanaPartyMarkerMenu (markers.Get (i).GetName (), markers.Get (i).GetPosition ()));
			}
        }
    }

    void Add (SchanaPartyMarkerInfo marker) {
		CurrentCount++;
        GetSchanaPartyMarkerSettings ().Add (marker.GetPosition ());
        markers.Insert (marker);
		if (markers.Count() > 0 && markers.Count() > GetSchanaPartyServerSettings ().GetMaxMarkers () && GetSchanaPartyServerSettings ().GetMaxMarkers () > 0){
			markers.RemoveOrdered(0);
		}
        ClientUpdatePartyMarkers (serverMarkers);
        Send ();
    }

    void Reset () {
		CurrentCount = 1;
        GetSchanaPartyMarkerSettings ().Clear ();
        markers.Clear ();
        ClientUpdatePartyMarkers (serverMarkers);
        Send ();
    }

    void Send (bool tryAgain = true) {
        if (canSend && markers) {
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
			return GetGame ().GetPlayer ().GetIdentity ().GetName () + " " + (CurrentCount).ToString ();
		}
			return "[NULL] " + (CurrentCount).ToString ();
    }
}

static ref SchanaPartyMarkerManagerClient g_SchanaPartyMarkerManagerClient;
static ref SchanaPartyMarkerManagerClient GetSchanaPartyMarkerManagerClient () {
    if (g_Game.IsClient () && !g_SchanaPartyMarkerManagerClient) {
        g_SchanaPartyMarkerManagerClient = new SchanaPartyMarkerManagerClient;
    }
    return g_SchanaPartyMarkerManagerClient;
}