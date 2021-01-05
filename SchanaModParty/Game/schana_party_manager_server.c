class SchanaPartyManagerServer {
	protected ref map<string, ref set<string>> configurations;
	protected bool canSendInfo = true;

	protected bool canGenerateParties = true;
	protected bool canGeneratePositions = true;
	protected bool canGenerateHealth = true;

	ref map<string, ref set<string>> parties;
	ref map<string, vector> player_positions;
	ref map<string, float> player_healths;

	void SchanaPartyManagerServer () {
		SchanaPartyUtils.LogMessage ("Server Init " + SCHANA_PARTY_VERSION);
		configurations = new ref map<string, ref set<string>> ();
		GetRPCManager ().AddRPC ("SchanaModParty", "ServerRegisterPartyRPC", this, SingleplayerExecutionType.Both);

		GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.SendInfo, 10000, true);
		GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.ResetSendInfoLock, GetSchanaPartyServerSettings ().GetSendInfoFrequency () * 1000, true);

		int logFrequency = GetSchanaPartyServerSettings ().GetLogFrequency ();
		if (logFrequency > 0) {
			GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.LogParties, logFrequency * 1000, true);
		}
	}

	void ~SchanaPartyManagerServer () {
		GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).Remove (this.SendInfo);
		GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).Remove (this.ResetSendInfoLock);

		int logFrequency = GetSchanaPartyServerSettings ().GetLogFrequency ();
		if (logFrequency > 0) {
			GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).Remove (this.LogParties);
		}
	}

	protected void ResetSendInfoLock () {
		canSendInfo = true;
	}

	protected void LogParties () {
		string result;
		JsonSerializer ().WriteToString (parties, false, result);
		SchanaPartyUtils.Warn ("Parties " + result);
	}

	void ServerRegisterPartyRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param2<string, ref array<string>> data;
		if (!ctx.Read (data))
			return;

		if (SchanaPartyUtils.WillLog (SchanaPartyUtils.INFO)) {
			string result;
			JsonSerializer ().WriteToString (data, false, result);
			SchanaPartyUtils.Info ("ServerRegisterPartyRPC " + result);
		}

		ServerRegisterParty (data.param1, data.param2);
	}

	protected void ServerRegisterParty (string key, ref array<string> ids) {
		SchanaPartyUtils.Info ("Register " + ids.Count ().ToString () + " to " + key);
		auto party_members = new ref set<string> ();
		foreach (string id : ids) {
			party_members.Insert (id);
		}

		string result;

		if (SchanaPartyUtils.WillLog (SchanaPartyUtils.TRACE)) {
			JsonSerializer ().WriteToString (configurations, false, result);
			SchanaPartyUtils.Trace ("ServerRegisterParty Before " + result);
		}

		configurations.Set (key, party_members);

		if (SchanaPartyUtils.WillLog (SchanaPartyUtils.TRACE)) {
			JsonSerializer ().WriteToString (configurations, false, result);
			SchanaPartyUtils.Trace ("ServerRegisterParty After " + result);
		}

		SendInfo ();
	}

	ref map<string, ref set<string>> GetParties () {
		if (!canGenerateParties) {
			return parties;
		}
		int maxPartyRefreshRate = GetSchanaPartyServerSettings ().GetMaxPartyRefreshRate ();
		GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.ResetPartiesRefreshRate, maxPartyRefreshRate * 1000, false);
		canGenerateParties = false;
		parties = new ref map<string, ref set<string>> ();

		foreach (auto owner_id, auto party_ids : configurations) {
			auto validated_party_ids = new ref set<string> ();

			foreach (string member_id : party_ids) {
				if (configurations.Contains (member_id) && configurations.Get (member_id).Find (owner_id) != -1) {
					validated_party_ids.Insert (member_id);
				} else if (configurations.Contains (member_id) && GetSchanaPartyServerSettings ().GetAdminIds ().Find (owner_id) != -1) {
					validated_party_ids.Insert (member_id);
				}
			}
			parties.Insert (owner_id, validated_party_ids);
		}

		return parties;
	}

	void ResetPartiesRefreshRate () {
		canGenerateParties = true;
	}

	ref array<DayZPlayer> GetPartyPlayers (string id) {
		map<string, DayZPlayer> id_map = new map<string, DayZPlayer> ();
		array<Man> game_players = new array<Man>;
		GetGame ().GetPlayers (game_players);
		int i;
		for (i = 0; i < game_players.Count (); ++i) {
			DayZPlayer player = DayZPlayer.Cast (game_players.Get (i));
			if (player && player.GetIdentity () && player.IsAlive ()) {
				id_map.Insert (player.GetIdentity ().GetId (), player);
			}
		}

		array<DayZPlayer> players = new array<DayZPlayer>;
		set<string> member_ids = GetParties ().Get (id);
		if (member_ids) {
			for (i = 0; i < member_ids.Count (); ++i) {
				string member_id = member_ids.Get (i);
				if (id_map.Contains (member_id)) {
					DayZPlayer plr = DayZPlayer.Cast (id_map.Get (member_id));
					if (plr) {
						players.Insert (plr);
					}
				}
			}
		}
		return players;
	}

	protected ref map<string, vector> GetPositions () {
		if (!canGeneratePositions) {
			return player_positions;
		}

		SchanaPartyUtils.Trace ("GetPositions Start ");
		player_positions = new ref map<string, vector> ();

		int maxPartyRefreshRate = GetSchanaPartyServerSettings ().GetMaxPartyRefreshRate ();
		GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.ResetPositionsRefreshRate, maxPartyRefreshRate * 1000, false);
		canGeneratePositions = false;

		array<Man> players = new array<Man>;
		GetGame ().GetPlayers (players);

            for (int i = 0; i < players.Count (); ++i ) {
                DayZPlayer player = DayZPlayer.Cast (players.Get (i));
                if (player && player.GetIdentity () && player.IsAlive ()) {
                    player_positions.Insert (player.GetIdentity ().GetId (), player.GetPosition ());
                }
            }
		SchanaPartyUtils.Trace ("GetPositions Finish");

		return player_positions;
	}

	void ResetPositionsRefreshRate () {
		canGeneratePositions = true;
	}

	protected ref map<string, float> GetHealths () {
		if (!canGenerateHealth) {
			return player_healths;
		}
		SchanaPartyUtils.Trace ("GetHealths Start ");
		player_healths = new ref map<string, float> ();

		int maxPartyRefreshRate = GetSchanaPartyServerSettings ().GetMaxPartyRefreshRate ();
		GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.ResetHealthsRefreshRate, maxPartyRefreshRate * 1000, false);
		canGenerateHealth = false;

		array<Man> players = new array<Man>;
		GetGame ().GetPlayers (players);

            for (int i = 0; i < players.Count (); ++i ) {
                DayZPlayer player = DayZPlayer.Cast (players.Get (i));
                if (player && player.GetIdentity () && player.IsAlive ()) {
                    player_healths.Insert (player.GetIdentity ().GetId (), player.GetHealth ("", ""));
                }
            }
		SchanaPartyUtils.Trace ("GetHealths Finish ");

		return player_healths;
	}

	void ResetHealthsRefreshRate () {
		canGenerateHealth = true;
	}

	protected void SendInfo () {
		if (canSendInfo) {
			thread SendInfoThread ();
			
			int sendInfoFrequency = GetSchanaPartyServerSettings ().GetSendInfoFrequency ();
			GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.ResetSendInfoLock, sendInfoFrequency * 1000, false);
			
			canSendInfo = false;
		}
	}

    protected void SendInfoThread () {

		SendPartyInfo ();
		SendPlayersInfo ();
	}


	protected void SendPartyInfo () {

		auto id_map = new ref map<string, DayZPlayer> ();

		array<Man> players = new array<Man>;
		GetGame ().GetPlayers (players);

            for (int i = 0; i < players.Count (); ++i ) {
                DayZPlayer player = DayZPlayer.Cast (players.Get (i));
                if (player && player.GetIdentity () && player.IsAlive ()) {
                    id_map.Insert (player.GetIdentity ().GetId (), player);
                }
            }
			
		auto positions = GetPositions ();
		auto server_healths = GetHealths ();
		auto s_parties = GetParties ();

		if (SchanaPartyUtils.WillLog (SchanaPartyUtils.DEBUG)) {
			string result;
			JsonSerializer ().WriteToString (positions, false, result);
			SchanaPartyUtils.Debug ("Positions " + result);
			JsonSerializer ().WriteToString (server_healths, false, result);
			SchanaPartyUtils.Debug ("Healths " + result);
			JsonSerializer ().WriteToString (s_parties, false, result);
			SchanaPartyUtils.Debug ("Parties " + result);
		}

		int maxPartySize = GetSchanaPartyServerSettings ().GetMaxPartySize ();
		int SendDelay = 1;
		foreach (auto id, auto party_ids : s_parties) {
			SendDelay++; //To help performance to devide up when the parties are all sent
			SchanaPartyUtils.Trace ("SendInfo Begin " + id);
			if (!positions.Contains (id)) {
				configurations.Remove (id);
			} else {
				DayZPlayer ply = DayZPlayer.Cast (id_map.Get (id));
				if (ply && ply.GetIdentity () && ply.IsAlive ()) {
					SendPartyInfoToPlayer (id, party_ids, maxPartySize, positions, server_healths, ply.GetIdentity ());
				}
			}
			SchanaPartyUtils.Trace ("SendInfo End " + id);
		}
	}

	protected void SendPartyInfoToPlayer (string id, ref set<string> party_ids, int maxPartySize, ref map<string, vector> positions, ref map<string, float> server_healths, PlayerIdentity player) {
		auto ids = new ref array<string>;
		auto locations = new ref array<vector>;
		auto healths = new ref array<float>;
		foreach (string party_id : party_ids) {
			if (positions.Contains (party_id) && (maxPartySize < 0 || ids.Count () < maxPartySize)) {
				ids.Insert (party_id);
				locations.Insert (positions.Get (party_id));
				healths.Insert (server_healths.Get (party_id));
			}
		}
		auto info = new ref Param3<ref array<string>, ref array<vector>, ref array<float>> (ids, locations, healths);

		if (SchanaPartyUtils.WillLog (SchanaPartyUtils.INFO)) {
			string result;
			JsonSerializer ().WriteToString (info, false, result);
			SchanaPartyUtils.Info ("SendInfo to " + id + " " + result);
		}

		PlayerIdentity plyr = PlayerIdentity.Cast (player);
		if (plyr) {
			GetRPCManager ().SendRPC ("SchanaModParty", "ClientUpdatePartyInfoRPC", info, false, plyr);
		} else {
			SchanaPartyUtils.Warn ("SendInfo failed to " + id);
		}
	}

	protected void SendPlayersInfo () {
		
		auto id_map = new ref map<string, DayZPlayer> ();
		auto all_player_ids = new ref array<string>;
		auto all_player_names = new ref array<string>;

		array<Man> players = new array<Man>;
		GetGame ().GetPlayers (players);

            for (int i = 0; i < players.Count (); ++i ) {
                DayZPlayer player = DayZPlayer.Cast (players.Get (i));
                if (player && player.GetIdentity () && player.IsAlive ()) {
					all_player_ids.Insert (player.GetIdentity ().GetId ());
					all_player_names.Insert (player.GetIdentity ().GetName ());
                }
            }
		
		auto all_player_info = new ref Param2<ref array<string>, ref array<string>> (all_player_ids, all_player_names);

		if (SchanaPartyUtils.WillLog (SchanaPartyUtils.DEBUG)) {
			string result;
			JsonSerializer ().WriteToString (all_player_info, false, result);
			SchanaPartyUtils.Debug ("SendPlayers " + result);
		}

		GetRPCManager ().SendRPC ("SchanaModParty", "ClientUpdatePlayersInfoRPC", all_player_info);
	}
}

static ref SchanaPartyManagerServer g_SchanaPartyManagerServer;
static ref SchanaPartyManagerServer GetSchanaPartyManagerServer () {
	if (g_Game.IsServer () && !g_SchanaPartyManagerServer) {
		g_SchanaPartyManagerServer = new SchanaPartyManagerServer;
	}
	return g_SchanaPartyManagerServer;
}
