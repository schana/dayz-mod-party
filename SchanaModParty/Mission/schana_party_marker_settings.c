class SchanaModPartyMarkerSettings {
    private static string DIR = "$profile:SchanaModParty";
    private static string PATH = DIR + "\\markers.json";

    private ref array<ref vector> markers;

    void SchanaModPartyMarkerSettings () {
        markers = new ref array<ref vector> ();
    }

    void Add (vector mark) {
        markers.Insert (mark);
        Save ();
    }

    void Clear () {
        markers.Clear ();
        Save ();
    }

    ref array<ref vector> GetMarkers () {
        auto response = new ref array<ref vector> ();

        for (int i = 0; i < markers.Count (); ++i) {
            response.Insert (markers.Get (i));
        }

        return response;
    }

    void Save () {
        if (GetGame ().IsClient ()) {
            if (!FileExist (DIR)) {
                MakeDirectory (DIR);
            }
            JsonFileLoader<SchanaModPartyMarkerSettings>.JsonSaveFile (PATH, this);
        }
    }

    static ref SchanaModPartyMarkerSettings Get () {
        auto settings = new ref SchanaModPartyMarkerSettings ();

        if (FileExist (PATH)) {
            JsonFileLoader<SchanaModPartyMarkerSettings>.JsonLoadFile (PATH, settings);
        }

        return settings;
    }
}

static ref SchanaModPartyMarkerSettings g_SchanaPartyMarkerSettings;
static ref SchanaModPartyMarkerSettings GetSchanaPartyMarkerSettings () {
    if (g_Game.IsClient () && !g_SchanaPartyMarkerSettings) {
        g_SchanaPartyMarkerSettings = SchanaModPartyMarkerSettings.Get ();
        g_SchanaPartyMarkerSettings.Save ();
    }
    return g_SchanaPartyMarkerSettings;
}