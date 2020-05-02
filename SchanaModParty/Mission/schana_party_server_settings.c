class SchanaModPartyServerSettings {
    private static string DIR = "$profile:SchanaModParty";
    private static string PATH = DIR + "\\server-config.json";

    private int logPartiesFrequencySeconds = 10;
    private int verbosity = 1;
    private int maxPartySize = -1;
    private int sendInfoFrequencySeconds = 2;

    int GetLogFrequency () {
        return logPartiesFrequencySeconds;
    }

    int GetVerbosity () {
        return verbosity;
    }

    int GetMaxPartySize () {
        return maxPartySize;
    }

    int GetSendInfoFrequency () {
        return sendInfoFrequencySeconds;
    }

    void Save () {
        if (GetGame ().IsServer ()) {
            if (!FileExist (DIR)) {
                MakeDirectory (DIR);
            }
            JsonFileLoader<SchanaModPartyServerSettings>.JsonSaveFile (PATH, this);
        }
    }

    static ref SchanaModPartyServerSettings Get () {

        auto settings = new ref SchanaModPartyServerSettings ();

        if (FileExist (PATH)) {
            JsonFileLoader<SchanaModPartyServerSettings>.JsonLoadFile (PATH, settings);
        } else {
            settings.Save ();
        }

        return settings;
    }
}

static ref SchanaModPartyServerSettings g_SchanaPartyServerSettings;
static ref SchanaModPartyServerSettings GetSchanaPartyServerSettings () {
    if (g_Game.IsServer () && !g_SchanaPartyServerSettings) {
        g_SchanaPartyServerSettings = SchanaModPartyServerSettings.Get ();
    }
    return g_SchanaPartyServerSettings;
}