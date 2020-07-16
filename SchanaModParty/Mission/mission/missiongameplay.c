modded class MissionGameplay extends MissionBase {
    private ref SchanaPartyMenu m_SchanaPartyMenu;

    override void OnInit () {
        super.OnInit ();
        delete g_SchanaPartyManagerClient;
        GetSchanaPartyManagerClient ();
        delete g_SchanaPartyMarkerManagerClient;
        GetSchanaPartyMarkerManagerClient ();
    }

    override void OnMissionFinish () {
        delete g_SchanaPartyManagerClient;
        delete g_SchanaPartyMarkerManagerClient;
        super.OnMissionFinish ();
    }

    override void OnUpdate (float timeslice) {
        super.OnUpdate (timeslice);

        if (GetUApi () && !m_UIManager.IsMenuOpen (MENU_CHAT_INPUT)) {
            Input input = GetGame ().GetInput ();
            if (input.LocalPress ("UAUIBack", false)) {
                if (m_SchanaPartyMenu != NULL && GetGame ().GetUIManager ().GetMenu () == m_SchanaPartyMenu) {
                    m_SchanaPartyMenu.SchanaPartyMenuSetOpen (false);
                    GetGame ().GetUIManager ().HideScriptedMenu (m_SchanaPartyMenu);
                    SchanaPartyUnlockControls ();
                }
            }

            if (input.LocalPress ("UASchanaPartyMenu", false)) {
                if (m_SchanaPartyMenu) {
                    if (m_SchanaPartyMenu.SchanaPartyMenuIsOpen ()) {
                        //Hide Menu
                        m_SchanaPartyMenu.SchanaPartyMenuSetOpen (false);
                        GetGame ().GetUIManager ().HideScriptedMenu (m_SchanaPartyMenu);
                        SchanaPartyUnlockControls ();
                    } else if (GetGame ().GetUIManager ().GetMenu () == NULL) {
                        //Show Menu
                        GetGame ().GetUIManager ().ShowScriptedMenu (m_SchanaPartyMenu, NULL);
                        m_SchanaPartyMenu.SchanaPartyMenuSetOpen (true);
                        SchanaPartyLockControls ();
                    }
                } else if (GetGame ().GetUIManager ().GetMenu () == NULL && m_SchanaPartyMenu == null) {
                    //Create Menu
                    SchanaPartyLockControls ();
                    m_SchanaPartyMenu = SchanaPartyMenu.Cast (GetUIManager ().EnterScriptedMenu (SCHANA_PARTY_MENU, null));
                    m_SchanaPartyMenu.SchanaPartyMenuSetOpen (true);
                }
            }

            if (input.LocalPress ("UASchanaPartyPing", false)) {
                vector position = SchanaPartyGetRaycastPosition ();
                if (position != vector.Zero) {
                    auto marker_client = GetSchanaPartyMarkerManagerClient ();
                    auto new_marker = new SchanaPartyMarkerInfo (marker_client.GetNextName (), position);
                    marker_client.Add (new_marker);
                }
            }

            if (input.LocalPress ("UASchanaPartyPingClear", false)) {
                GetSchanaPartyMarkerManagerClient ().Reset ();
            }
        }
    }

    private vector SchanaPartyGetRaycastPosition () {
        vector begin = GetGame ().GetCurrentCameraPosition () + GetGame ().GetCurrentCameraDirection ();
        vector end = begin + GetGame ().GetCurrentCameraDirection () * 8000;
        vector contactPos;
        vector contactDir;
        int contactComponent;

        if (DayZPhysics.RaycastRV (begin, end, contactPos, contactDir, contactComponent)) {
            return contactPos;
        }

        return vector.Zero;
    }

    private void SchanaPartyLockControls () {
        GetGame ().GetMission ().PlayerControlDisable (INPUT_EXCLUDE_MOUSE_ALL);
        GetGame ().GetUIManager ().ShowUICursor (true);
    }

    private void SchanaPartyUnlockControls () {
        GetGame ().GetMission ().PlayerControlEnable (false);
        GetGame ().GetInput ().ResetGameFocus ();
        GetGame ().GetUIManager ().ShowUICursor (false);
    }
}

static ref SchanaPartyManagerClient g_SchanaPartyManagerClient;
static ref SchanaPartyManagerClient GetSchanaPartyManagerClient () {
    if (!g_Game.IsServer () && !g_SchanaPartyManagerClient) {
        g_SchanaPartyManagerClient = new SchanaPartyManagerClient;
    }
    return g_SchanaPartyManagerClient;
}