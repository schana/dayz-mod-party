class SchanaModPartyClientDisplaySettings {
    private static string DIR = "$profile:SchanaModParty";
    private static string PATH = DIR + "\\display_settings.json";

    private bool enable_party = true;
    private bool show_markers = true;

    private vector nametag_color = "255 255 255";
    private vector distance_color = "255 255 255";
    private vector icon_color = "255 255 255";

    private bool nametag_enabled = true;
    private int nametag_distance_start = 0;
    private int nametag_distance_end = 15000;

    private bool distance_enabled = true;
    private int distance_distance_start = 0;
    private int distance_distance_end = 2000;

    private bool icon_enabled = true;
    private int icon_distance_start = 0;
    private int icon_distance_end = 15000;

    private int snap_distance = 2000;

    void Save () {
        if (GetGame ().IsClient ()) {
            if (!FileExist (DIR)) {
                MakeDirectory (DIR);
            }
            JsonFileLoader<SchanaModPartyClientDisplaySettings>.JsonSaveFile (PATH, this);
        }
    }

    static ref SchanaModPartyClientDisplaySettings Get () {

        auto settings = new ref SchanaModPartyClientDisplaySettings ();

        if (FileExist (PATH)) {
            JsonFileLoader<SchanaModPartyClientDisplaySettings>.JsonLoadFile (PATH, settings);
        }

        return settings;
    }
}

static ref SchanaModPartyClientDisplaySettings g_SchanaModPartyClientDisplaySettings;
static ref SchanaModPartyClientDisplaySettings GetSchanaModPartyClientDisplaySettings () {
    if (g_Game.IsClient () && !g_SchanaModPartyClientDisplaySettings) {
        g_SchanaModPartyClientDisplaySettings = SchanaModPartyClientDisplaySettings.Get ();
        g_SchanaModPartyClientDisplaySettings.Save ();
    }
    return g_SchanaModPartyClientDisplaySettings;
}