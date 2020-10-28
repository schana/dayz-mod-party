class SchanaPartyBasicMapAPI {
    static const string GROUP_ID_LOCAL = "schanamodparty_local";
    static const string GROUP_ID_REMOTE = "schanamodparty_remote";

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

    void ClientInit () {
#ifdef BASICMAP
        controller = BasicMap ();
        BasicMapGroupMetaData localMeta = new BasicMapGroupMetaData (GROUP_ID_LOCAL, "Party", true);
        BasicMapGroupMetaData remoteMeta = new BasicMapGroupMetaData (GROUP_ID_REMOTE, "Party", false);
        controller.RegisterGroup (GROUP_ID_LOCAL, localMeta, SchanaPartyLocalMarkerFactory ());
        controller.RegisterGroup (GROUP_ID_REMOTE, remoteMeta, SchanaPartyRemoteMarkerFactory ());

        //register rpc for adding remote markers
        //start periodic publish

#endif
    }

    void ServerInit () {
#ifdef BASICMAP
        //register rpc for receiving local markers
#endif
    }

#ifdef BASICMAP
    void AddMarker (string group, string name, vector pos) {
        controller.CreateMarker (group, name, pos);
    }

    bool MarkerExists (vector position) {
        float distance = 0.1;
        if (controller.GetMarkerByVector (position, distance)) {
            return true;
        }
        return false;
    }

    void RemoveMarker (vector position) {
        float distance = 0.1;
        controller.RemoveMarkerByVector (position, distance)
        controller.RemoveAllMarkers (group);
    }
#endif
}