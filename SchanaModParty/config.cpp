class CfgPatches
{
	class SchanaModParty
	{
		requiredAddons[] = {};
		units[] = {};
		weapons[] = {};
	};
};

class CfgMods
{
	class SchanaModParty
	{
		name = "SchanaModParty";
		action = "https://github.com/schana/dayz-mod-party";
		author = "schana";
		type = "mod";
		inputs = "SchanaModParty/Data/Inputs.xml";
		dependencies[] =
		{
			"Game",
			"Mission"
		};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {
					"SchanaModParty/Game"
				};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {
					"SchanaModParty/Mission"
				};
			};
		};
	};
};