class SchanaModPartySettings
{
	private static string DIR = "$profile:SchanaModParty";
	private static string PATH = DIR + "\\config.json";
    private static bool initialized = false;
    private static ref SchanaModPartySettings settings;

    private ref map<string, string> players;

    void Add(string id, string name)
    {
        players.Set(id, name);
        Save();
    }

    void Remove(string id)
    {
        players.Remove(id);
        Save();
    }

    string GetName(string id)
    {
        return players.Get(id);
    }

    bool Contains(string id)
    {
        return players.Contains(id);
    }

    private void Save()
    {
        if (GetGame().IsClient())
        {
            MakeDirectory(DIR);
            JsonFileLoader<SchanaModPartySettings>.JsonSaveFile(PATH, this);
        }
    }

    static ref SchanaModPartySettings Get()
    {
        if (initialized)
        {
            return settings;
        }

        ref SchanaModPartySettings data = new ref SchanaModPartySettings;

        if (FileExist(PATH))
        {
            JsonFileLoader<SchanaModPartySettings>.JsonLoadFile(PATH, data);
            initialized = true;
        }
        else
        {
            if (GetGame().IsClient())
            {
                MakeDirectory(DIR);
                JsonFileLoader<SchanaModPartySettings>.JsonSaveFile(PATH, data);
            }
        }

        settings = data;

        return settings;
    }
}
