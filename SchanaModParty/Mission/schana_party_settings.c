class SchanaModPartySettings {
    private static string DIR = "$profile:SchanaModParty";
    private static string PATH = DIR + "\\config.json";

    private ref map<ref string, ref string> players;

    void SchanaModPartySettings () {
        players = new ref map<ref string, ref string> ();
    }

    void Add (string id, string name) {
        players.Set (id, name);
        Save ();
    }

    void Remove (string id) {
        players.Remove (id);
        Save ();
    }

    string GetName (string id) {
        return players.Get (id);
    }

    bool Contains (string id) {
        return players.Contains (id);
    }

    ref array<ref string> GetMembers () {
        auto members = new ref array<ref string> ();

        foreach (string key, string item : players) {
            members.Insert (key);
        }

        return members;
    }

    void Save () {
        if (GetGame ().IsClient ()) {
            if (!FileExist (DIR)) {
                MakeDirectory (DIR);
            }
            JsonFileLoader<SchanaModPartySettings>.JsonSaveFile (PATH, this);
        }
    }

    static ref SchanaModPartySettings Get () {

        auto settings = new ref SchanaModPartySettings ();

        if (FileExist (PATH)) {
            JsonFileLoader<SchanaModPartySettings>.JsonLoadFile (PATH, settings);
        }

        return settings;
    }
}

static ref SchanaModPartySettings g_SchanaPartySettings;
static ref SchanaModPartySettings GetSchanaPartySettings () {
    if (g_Game.IsClient () && !g_SchanaPartySettings) {
        g_SchanaPartySettings = SchanaModPartySettings.Get ();
        g_SchanaPartySettings.Save ();
    }
    return g_SchanaPartySettings;
}