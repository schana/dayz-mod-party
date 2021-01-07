class SchanaModPartySettings {
    protected static string DIR = "$profile:SchanaModParty";
    protected static string PATH = DIR + "\\config_1.json";

    protected ref array<string> players;
    protected ref array<string> names;

    void SchanaModPartySettings () {
        players = new ref array<string> ();
        names = new ref array<string> ();
    }

    void Add (string id, string name) {
        int index = players.Find (id);
        if (index == -1) {
            players.Insert (id);
            names.Insert (name);
        } else {
            names.Set (index, name);
        }
        Save ();
    }

    void Remove (string id) {
        int index = players.Find (id);
        if (index != -1) {
            players.Remove (index);
            names.Remove (index);
        }
        Save ();
    }

    string GetName (string id) {
        int index = players.Find (id);
        if (index != -1) {
            return names.Get (index);
        }
        return "";
    }

    bool Contains (string id) {
        int index = players.Find (id);
        return index != -1;
    }

    ref array<string> GetMembers () {
        auto members = new ref array<string> ();

        for (int i = 0; i < players.Count (); ++i) {
            members.Insert (players.Get (i));
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