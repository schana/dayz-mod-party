class SchanaPartyManagerServer {
	private ref map<string, ref set<string>> configurations;
	private bool canSendInfo = true;

	private bool canGenerateParties = true;
	private bool canGeneratePositions = true;
	private bool canGenerateHealth = true;

	private ref map<string, ref set<string>> parties;
	private ref map<string, vector> player_positions;
	private ref map<string, float> player_healths;
	private auto validated_party_ids;
	private auto party_members;

	private ref map<string, DayZPlayer> id_map;
	private ref array<Man> game_players;
	private ref array<DayZPlayer> players;
	private ref set<string> member_ids;

	void SchanaPartyManagerServer () {
		SchanaPartyUtils.LogMessage ("Server Init " + SCHANA_PARTY_VERSION);
		configurations = new ref map<string, ref set<string>> ();
		parties = new ref map<string, ref set<string>> ();
		player_positions = new ref map<string, vector> ();
		player_healths = new ref map<string, float> ();
		validated_party_ids = new ref set<string> ();
		party_members = new ref set<string> ();
		id_map  = new ref map<string, DayZPlayer> ();
		game_players  = new array<Man>;
		players = new ref array<DayZPlayer>;
		member_ids = null;
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

	private void ResetSendInfoLock () {
		canSendInfo = true;
	}

	private void LogParties () {
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

	private void ServerRegisterParty (string key, ref array<string> ids) {
		SchanaPartyUtils.Info ("Register " + ids.Count ().ToString () + " to " + key);
		party_members.Clear ();
		foreach (string id : ids) {
			if (!party_members.Contains(id)) {
				party_members.Insert (id);
			}
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

		foreach (auto owner_id, auto party_ids : configurations) {

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

	private ref void SetPartyPlayers (string id) {
		GetGame ().GetPlayers (game_players);
		id_map.Clear ();
		foreach (Man man : game_players) {
			DayZPlayer player = DayZPlayer.Cast (man);
			if (player && player.GetIdentity () && player.IsAlive ()) {
				id_map.Insert (player.GetIdentity ().GetId (), player);
			}
		}

		member_ids = GetParties ().Get (id);
		if (member_ids) {
			foreach (string member_id : member_ids) {
				if (id_map.Contains (member_id)) {
					DayZPlayer plr = DayZPlayer.Cast (id_map.Get (member_id));
					if (plr && !players.Contains(plr)) {
						players.Insert (plr);
					}
				}
			}
		}
	}

	protected ref array<DayZPlayer> GetPartyPlayers (string id) {
		return players;
	}

	private ref void SetPositions () {
		if (!canGeneratePositions) {
			return;
		}

		int maxPartyRefreshRate = GetSchanaPartyServerSettings ().GetMaxPartyRefreshRate ();
		GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.ResetPositionsRefreshRate, maxPartyRefreshRate * 1000, false);
		canGeneratePositions = false;
		
		GetGame ().GetPlayers (players);
		player_positions.Clear ();
		foreach (Man man : players) {
			DayZPlayer player = DayZPlayer.Cast (man);
			if (player && player.GetIdentity () && player.IsAlive ()) {
				player_positions.Insert (player.GetIdentity ().GetId (), player.GetPosition ());
			}
		}
	}

	private ref map<string, vector> GetPositions () {
		return player_positions;
	}

	void ResetPositionsRefreshRate () {
		canGeneratePositions = true;
	}

	private ref void SetHealths () {
		if (!canGenerateHealth) {
			return;
		}

		int maxPartyRefreshRate = GetSchanaPartyServerSettings ().GetMaxPartyRefreshRate ();
		GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.ResetHealthsRefreshRate, maxPartyRefreshRate * 1000, false);
		canGenerateHealth = false;

		GetGame ().GetPlayers (players);
		player_healths.Clear ();
		foreach (Man man : players) {
			DayZPlayer player = DayZPlayer.Cast (man);
			float health;
			if (player && player.GetIdentity () && player.IsAlive ()) {
				player_healths.Insert (player.GetIdentity ().GetId (), player.GetHealth ("", ""));
			}
			
		}
	}

	private ref map<string, float> GetHealths () {
		return player_healths;
	}

	void ResetHealthsRefreshRate () {
		canGenerateHealth = true;
	}

	private void SendInfo () {
		if (canSendInfo) {
			GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).Call (this.SendPartyInfo, id_map);
			GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).Call (this.SendPlayersInfo, id_map);
			int sendInfoFrequency = GetSchanaPartyServerSettings ().GetSendInfoFrequency ();
			GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.ResetSendInfoLock, sendInfoFrequency * 1000, false);

			canSendInfo = false;
		}
	}

	private void SendPartyInfo (ref map<string, DayZPlayer> id_map) {

		if (SchanaPartyUtils.WillLog (SchanaPartyUtils.DEBUG)) {
			string result;
			JsonSerializer ().WriteToString (positions, false, result);
			SchanaPartyUtils.Debug ("Positions " + result);
			JsonSerializer ().WriteToString (server_healths, false, result);
			SchanaPartyUtils.Debug ("Healths " + result);
			JsonSerializer ().WriteToString (parties, false, result);
			SchanaPartyUtils.Debug ("Parties " + result);
		}

		int maxPartySize = GetSchanaPartyServerSettings ().GetMaxPartySize ();
		int SendDelay = 1;
		foreach (auto id, auto party_ids : parties) {
			SendDelay++; //To help performance to devide up when the parties are all sent
			SchanaPartyUtils.Trace ("SendInfo Begin " + id);
			if (!positions.Contains (id)) {
				configurations.Remove (id);
			} else {
				DayZPlayer player = DayZPlayer.Cast (id_map.Get (id));
				if (player) {
					GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.SendPartyInfoToPlayer, SendDelay, false, id, party_ids, maxPartySize, positions, server_healths, player);
				}
			}
			SchanaPartyUtils.Trace ("SendInfo End " + id);
		}
	}

	private void SendPartyInfoToPlayer (string id, ref set<string> party_ids, int maxPartySize, ref map<string, vector> positions, ref map<string, float> server_healths, DayZPlayer player) {
		auto ids = new ref array<string>;
		auto locations = new ref array<vector>;
		auto healths = new ref array<float>;
		foreach (string party_id : party_ids) {
			if (positions.Contains (party_id) && (maxPartySize < 0 || party_ids.Count () < maxPartySize)) {
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

		DayZPlayer plyr = DayZPlayer.Cast (player);
		if (plyr && plyr.GetIdentity () && plyr.IsAlive ()) {
			GetRPCManager ().SendRPC ("SchanaModParty", "ClientUpdatePartyInfoRPC", info, false, plyr.GetIdentity ());
		} else {
			SchanaPartyUtils.Warn ("SendInfo failed to " + id);
		}
	}

	private void SendPlayersInfo (ref map<string, DayZPlayer> id_map) {
		auto all_player_ids = new ref array<string>;
		auto all_player_names = new ref array<string>;
		foreach (auto player_id, auto player_base_player : id_map) {
			DayZPlayer plyr = DayZPlayer.Cast (player_base_player);
			if (plyr && plyr.IsAlive ()){
				PlayerIdentity theIdentity = PlayerIdentity.Cast (plyr.GetIdentity ());
				if (theIdentity){
					all_player_ids.Insert (player_id);
					all_player_names.Insert (theIdentity.GetName ());
				}
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
