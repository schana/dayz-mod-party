class SchanaPartyBasicMapAPI {
    static const string GROUP_ID_LOCAL = "schanamodparty_local";
    static const string GROUP_ID_REMOTE = "schanamodparty_remote";
    static const string GROUP_ID_MEMBERS = "schanamodparty_members";
    static const string DISPLAY_NAME_LOCAL = "MY PARTY MARKERS";
    static const string DISPLAY_NAME_REMOTE = "PARTY MEMBER MARKERS";
    static const string DISPLAY_NAME_MEMBERS = "PARTY MEMBERS";

    private static ref SchanaPartyBasicMapAPI api;

    static SchanaPartyBasicMapAPI Get () {
        if (!api) {
            api = new ref SchanaPartyBasicMapAPI;
            if (GetGame ().IsClient ()) {
                api.ClientInit ();
            } else {
                api.ServerInit ();
            }
        }
        return api;
    }

#ifdef BASICMAP
    private ref BasicMapController controller;
#endif

    private void SchanaPartyBasicMapAPI () { }

#ifdef BASICMAP
    void ~SchanaPartyBasicMapAPI () {
        if (GetGame ().IsClient ()) {
            GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).Remove (this.SendMarkers);
        }
    }
#endif

    void ClientInit () {
#ifdef BASICMAP
		Print("[SchanaModParty] SchanaPartyBasicMapAPI - ClientInit"); //Making sure that the ifdef found
        controller = BasicMap ();
        BasicMapGroupMetaData remoteMeta = new BasicMapGroupMetaData (GROUP_ID_MEMBERS, DISPLAY_NAME_MEMBERS, false); //Registering First so they are first on the list?
        BasicMapGroupMetaData localMeta = new BasicMapGroupMetaData (GROUP_ID_LOCAL, DISPLAY_NAME_LOCAL, true);
        BasicMapGroupMetaData partyMeta = new BasicMapGroupMetaData (GROUP_ID_REMOTE, DISPLAY_NAME_REMOTE, false);
        controller.RegisterGroup (GROUP_ID_LOCAL, localMeta, SchanaPartyLocalMarkerFactory ());
        controller.RegisterGroup (GROUP_ID_REMOTE, remoteMeta, SchanaPartyRemoteMarkerFactory ());

        GetRPCManager ().AddRPC ("SchanaModParty", "ClientRegisterBasicMapMarkersRPC", this, SingleplayerExecutionType.Both);
        GetRPCManager ().AddRPC ("SchanaModParty", "ClientAddBasicMapMarkerRPC", this, SingleplayerExecutionType.Both);
        GetRPCManager ().AddRPC ("SchanaModParty", "ClientRemoveBasicMapMarkerRPC", this, SingleplayerExecutionType.Both);
        GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.SendMarkers, 30000, true);
#endif
    }

    void ServerInit () {
#ifdef BASICMAP
		Print("[SchanaModParty] SchanaPartyBasicMapAPI - ServerInit"); //Making sure that the ifdef found
        GetRPCManager ().AddRPC ("SchanaModParty", "ServerRegisterBasicMapMarkersRPC", this, SingleplayerExecutionType.Both);
        GetRPCManager ().AddRPC ("SchanaModParty", "ServerAddBasicMapMarkerRPC", this, SingleplayerExecutionType.Both);
        GetRPCManager ().AddRPC ("SchanaModParty", "ServerRemoveBasicMapMarkerRPC", this, SingleplayerExecutionType.Both);
#endif
    }

	void AddOrUpdatePlayerMarker(DayZPlayer thePlayer){
		#ifdef BASICMAP
			
		#endif
	}

#ifdef BASICMAP

    void ClientRegisterBasicMapMarkersRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        Param1<ref array<ref BasicMapMarker>> data;
        if (!ctx.Read (data))
            return;

        auto markers = data.param1;
        int i;

        for (i = 0; i < markers.Count (); ++i) {
            ClientAddBasicMapMarker (markers[i].GetName (), markers[i].GetPosition ());
        }
    }

    void ClientAddBasicMapMarkerRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        Param2<string, vector> data;
        if (!ctx.Read (data))
            return;

        ClientAddBasicMapMarker (data.param1, data.param2);
    }

    void ClientRemoveBasicMapMarkerRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        Param1<vector> data;
        if (!ctx.Read (data))
            return;

        ClientRemoveBasicMapMarker (data.param1);
    }

    void ClientAddBasicMapMarker (string name, vector position) {
        AddOrUpdateMarker (GROUP_ID_REMOTE, name, position);
    }

    void ClientRemoveBasicMapMarker (vector position) {
        RemoveMarker (position);
    }

    void ServerRegisterBasicMapMarkersRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        Param1<ref array<ref BasicMapMarker>> data;
        if (!ctx.Read (data))
            return;

        string id = sender.GetId ();

        ServerRegisterBasicMapMarkers (id, data);
    }

    void ServerRegisterBasicMapMarkers (string id, Param1<ref array<ref BasicMapMarker>> data) {
        auto manager = GetSchanaPartyManagerServer ();

        foreach (auto player : manager.GetPartyPlayers (id)) {
			DayZPlayer ply = DayZPlayer.Cast(player);
			if (ply && ply.GetIdentity() ){
				GetRPCManager ().SendRPC ("SchanaModParty", "ClientRegisterBasicMapMarkersRPC", data, false, ply.GetIdentity());
			}
        }
    }

    void ServerAddBasicMapMarkerRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        Param2<string, vector> data;
        if (!ctx.Read (data))
            return;

        string id = sender.GetId ();
        auto manager = GetSchanaPartyManagerServer ();

        foreach (auto player : manager.GetPartyPlayers (id)) {
			DayZPlayer ply = DayZPlayer.Cast(player);
			if (ply && ply.GetIdentity() ){
				GetRPCManager ().SendRPC ("SchanaModParty", "ClientAddBasicMapMarkerRPC", data, false, ply.GetIdentity());
			}
        }
    }

    void ServerRemoveBasicMapMarkerRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
        Param1<vector> data;
        if (!ctx.Read (data))
            return;

        string id = sender.GetId ();
        auto manager = GetSchanaPartyManagerServer ();

        foreach (auto player : manager.GetPartyPlayers (id)) {
			DayZPlayer ply = DayZPlayer.Cast(player);
			if (ply && ply.GetIdentity() ){
				GetRPCManager ().SendRPC ("SchanaModParty", "ClientRemoveBasicMapMarkerRPC", data, false, ply.GetIdentity());
			}
        }
    }

    void SendMarkers () {
        ref array<ref BasicMapMarker> markers = controller.GetMarkers (GROUP_ID_LOCAL);
        auto data = new Param1<ref array<ref BasicMapMarker>> (markers);
        GetRPCManager ().SendRPC ("SchanaModParty", "ServerRegisterBasicMapMarkersRPC", data);
    }

    void SendCreatedMarker (ref BasicMapMarker marker) {
        auto data = new Param2<string, vector> (marker.GetName (), marker.GetPosition ());
        GetRPCManager ().SendRPC ("SchanaModParty", "ServerAddBasicMapMarkerRPC", data);
    }

    void SendDeletedMarker (ref BasicMapMarker marker) {
        auto data = new Param1<vector> (marker.GetPosition ());
        GetRPCManager ().SendRPC ("SchanaModParty", "ServerRemoveBasicMapMarkerRPC", data);
    }

    void AddMarker (string group, string name, vector position) {
        controller.CreateMarker (group, name, position, true);
    }

    void AddOrUpdateMarker (string group, string name, vector position) {
        float distance = 0.1;
        BasicMapMarker marker = controller.GetMarkerByVector (position, distance, true);
        if (marker) {
            marker.Name = name;
        } else {
            AddMarker (group, name, position);
        }
    }

    void RemoveMarker (vector position) {
        float distance = 0.1;
        controller.RemoveMarkerByVector (position, distance, true);
    }
#endif
}