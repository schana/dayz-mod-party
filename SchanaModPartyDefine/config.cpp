class CfgPatches
{
	class SchanaModPartyDefine
	{
		requiredVersion=0.1;
		requiredAddons[]={
		};
	};
};

class CfgMods
{
	class SchanaModPartyDefine
	{
		dir="SchanaModPartyDefine";
        name="SchanaModPartyDefine";
        credits="schana, daemonforge";
        author="schana";
        authorID="0";
        version="0.1";
        extra=0;
        type="mod";
	    dependencies[]={ "Game", "World", "Mission"};
	    class defs
	    {
			class gameScriptModule
            {
				value = "";
                files[]={
					"SchanaModPartyDefine/scripts/Common"
					};
            };
			
			class worldScriptModule
            {
                value="";
                files[]={ 
					"SchanaModPartyDefine/scripts/Common"
				};
            };
			
	        class missionScriptModule
            {
                value="";
                files[]={
					"SchanaModPartyDefine/scripts/Common"
				};
            };
        };
    };
};