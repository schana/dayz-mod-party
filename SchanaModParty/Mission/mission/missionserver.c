modded class MissionServer extends MissionBase {
    private ref SchanaPartyManagerServer g_PartyManagerServer;
    private ref SchanaPartyMarkerManagerServer g_PartyMarkerManagerServer;

    override void OnInit () {
        super.OnInit ();
        g_PartyManagerServer = new GetSchanaPartyManagerServer ();
        g_PartyMarkerManagerServer = new GetSchanaPartyMarkerManagerServer ();
    }

    override void OnUpdate (float timeslice) {
        super.OnUpdate (timeslice);
        g_PartyManagerServer.SetPartyPlayers ();
        g_PartyManagerServer.SetPositions ();
        g_PartyManagerServer.SetHealths ();
    }

    override void OnMissionFinish () {
        delete g_PartyManagerServer;
        delete g_PartyMarkerManagerServer;
        super.OnMissionFinish ();
    }
}