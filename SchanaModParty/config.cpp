class CfgPatches
{
	class SchanaModParty
	{
		requiredAddons[] =
		{
			"DZ_Data"
		};
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

		class defs
		{
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
