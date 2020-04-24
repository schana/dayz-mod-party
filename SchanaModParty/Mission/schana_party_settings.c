class SchanaModPartySettings
{
	private static string DIR = "$profile:SchanaModParty";
	private static string PATH = DIR + "\\config.json";
    private static bool initialized = false;
    private static ref SchanaModPartySettings settings;

    private ref map<string, string> players;
	
	void SchanaModPartySettings()
	{
		players = new map<string, string>();
	}

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
	
	ref array<ref string> GetMembers()
	{
		string result;
		JsonSerializer().WriteToString(players, false, result);
		Print("[SchanaParty] SettingsGetMembers Players " + result);
		
		auto members = new ref array<ref string>();
		foreach (string key, string item : players)
		{
			members.Insert(key);
		}
		
		JsonSerializer().WriteToString(members, false, result);
		Print("[SchanaParty] SettingsGetMembers Members " + result);
		
		return members;
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

        ref SchanaModPartySettings data = new SchanaModPartySettings();

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
