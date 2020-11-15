class SchanaPartyNametagsMenu extends UIScriptedMenu {
    static string SCHANA_PARTY_NAMETAG_DELETE = "SCHANA_PARTY_NAMETAG_DELETE";

    private Widget m_SchanaPartyNametagRoot;
    private TextWidget m_SchanaPartyNametagNametag;
    private TextWidget m_SchanaPartyNametagDistance;
    private ImageWidget m_SchanaPartyNametagIcon;

    private Widget m_SchanaPartyListRootWidget;
    private ref array<ImageWidget> m_SchanaPartyListHealthWidgets;
    private TextWidget m_SchanaPartyListTextWidget;

    private DayZPlayer m_SchanaPartyNametagPlayer;
    private ref SchanaPartyMemberBasicMapMarkerHelper m_BasicMapHelper;
    private vector m_SchanaPartyPlayerServerPosition = "0 0 0";
    private float m_SchanaPartyPlayerServerHealth = 100;
    private string m_SchanaPartyPlayerName = "";
    private int m_SchanaPartyListIndex = 0;

    void SchanaPartyNametagsMenu (DayZPlayer player) {
        m_SchanaPartyNametagRoot = GetGame ().GetWorkspace ().CreateWidgets ("SchanaModParty/GUI/Layouts/nametag.layout");
        m_SchanaPartyNametagNametag = TextWidget.Cast (m_SchanaPartyNametagRoot.FindAnyWidget ("nametag"));
        m_SchanaPartyNametagDistance = TextWidget.Cast (m_SchanaPartyNametagRoot.FindAnyWidget ("distance"));
        m_SchanaPartyNametagIcon = ImageWidget.Cast (m_SchanaPartyNametagRoot.FindAnyWidget ("icon"));
        m_SchanaPartyNametagRoot.Show (false);

        m_SchanaPartyListRootWidget = GetGame ().GetWorkspace ().CreateWidgets ("SchanaModParty/GUI/Layouts/party.layout");
        m_SchanaPartyListTextWidget = TextWidget.Cast (m_SchanaPartyListRootWidget.FindAnyWidget ("Nametag"));
        m_SchanaPartyListHealthWidgets = new array<ImageWidget>;
        for (int i = 0; i < 5; ++i) {
            m_SchanaPartyListHealthWidgets.Insert (ImageWidget.Cast (m_SchanaPartyListRootWidget.FindAnyWidget ("IconHealth" + i.ToString ())));
        }

        m_SchanaPartyNametagPlayer = player;
        m_BasicMapHelper = new SchanaPartyMemberBasicMapMarkerHelper ();

        GetGame ().GetCallQueue (CALL_CATEGORY_GUI).CallLater (this.SchanaUpdate, 16, true);
    }

    void ~SchanaPartyNametagsMenu () {
        GetGame ().GetCallQueue (CALL_CATEGORY_GUI).Remove (this.SchanaUpdate);
        if (m_SchanaPartyNametagRoot != null) {
            m_SchanaPartyNametagRoot.Show (false);
            m_SchanaPartyNametagRoot.Unlink ();
        }

        if (m_SchanaPartyListRootWidget != null) {
            m_SchanaPartyListRootWidget.Show (false);
            m_SchanaPartyListRootWidget.Unlink ();
        }
        if (m_BasicMapHelper) {
            delete m_BasicMapHelper;
        }
    }

    void SchanaUpdate () {
        if (SchanaPartyNametagShouldShow ()) {
            SchanaPartyNametagUpdate ();
        } else if (m_SchanaPartyNametagRoot != null) {
            m_SchanaPartyNametagRoot.Show (false);
            delete this;
        }
    }

    private bool SchanaPartyNametagShouldShow () {
        if (m_SchanaPartyNametagRoot == null) {
            return false;
        }
        if (!GetGame ().GetPlayer ()) {
            return false;
        }
        if (m_SchanaPartyPlayerName == SCHANA_PARTY_NAMETAG_DELETE) {
            return false;
        }
        return true;
    }

    private vector SchanaPartyGetPlayerPosition () {
        if (m_SchanaPartyNametagPlayer && m_SchanaPartyNametagPlayer.IsAlive ()) {
            return m_SchanaPartyNametagPlayer.GetPosition ();
        } else {
            return m_SchanaPartyPlayerServerPosition;
        }
    }

    private float SchanaPartyGetPlayerHealth () {
        return m_SchanaPartyPlayerServerHealth;
    }

    void SchanaPartyUpdatePosition (vector position) {
        m_SchanaPartyPlayerServerPosition = position;
    }

    void SchanaPartyUpdateHealth (float health) {
        m_SchanaPartyPlayerServerHealth = health;
    }

    void SchanaPartyUpdatePlayer (DayZPlayer player) {
        m_SchanaPartyNametagPlayer = player;
    }

    void SchanaPartyUpdateName (string name) {
        m_SchanaPartyPlayerName = name;
    }

    void SchanaPartyUpdateListIndex (int index) {
        m_SchanaPartyListIndex = index;
    }

    void SchanaPartySetRemoveFlag () {
        m_SchanaPartyPlayerName = SCHANA_PARTY_NAMETAG_DELETE;
    }

    private string SchanaPartyGetPlayerName () {
        if (m_SchanaPartyNametagPlayer && m_SchanaPartyNametagPlayer.GetIdentity ()) {
            return m_SchanaPartyNametagPlayer.GetIdentity ().GetName ();
        } else {
            return m_SchanaPartyPlayerName;
        }
    }

    void SchanaPartyNametagUpdate () {
        float x, y, distance;
        vector position = SchanaPartyGetPlayerPosition ();
        vector screenPosition = GetGame ().GetScreenPos (position + "0 1.3 0");

        x = Math.Round (screenPosition[0]);
        y = Math.Round (screenPosition[1]);
        m_SchanaPartyNametagRoot.SetPos (x, y);

        distance = Math.Round (vector.Distance (position, GetGame ().GetPlayer ().GetPosition ()));
        string distanceString = distance.ToString () + "m";
        if (distance > 1000) {
            distanceString = (Math.Round (distance / 100) / 10).ToString () + "km";
        }
        string text = SchanaPartyGetPlayerName () + " " + distanceString;
        m_SchanaPartyNametagNametag.SetText (SchanaPartyGetPlayerName ());
        m_SchanaPartyNametagDistance.SetText (distanceString);

        SchanaPartyListUpdate (text);

        m_SchanaPartyNametagRoot.Show (SchanaPartyNametagVisibleOnScreen ());

        m_BasicMapHelper.SetPosition (position);
        m_BasicMapHelper.SetName (SchanaPartyGetPlayerName ());
    }

    void SchanaPartyListUpdate (string text) {
        m_SchanaPartyListTextWidget.SetText (text);
        float health = SchanaPartyGetPlayerHealth () * 0.01;
        int healthLevel = 4 - health * 4;
        for (int i = 0; i < 5; ++i) {
            m_SchanaPartyListHealthWidgets[i].Show (healthLevel == i);
        }
        float width, height, x, y;
        m_SchanaPartyListRootWidget.GetSize (width, height);
        m_SchanaPartyListRootWidget.GetPos (x, y);
        y = (5 + height) * m_SchanaPartyListIndex;

        m_SchanaPartyListRootWidget.SetPos (x, y);
    }

    private bool SchanaPartyNametagVisibleOnScreen () {
        vector position = SchanaPartyGetPlayerPosition ();
        vector screenPositionRelative = GetGame ().GetScreenPosRelative (position);

        if (screenPositionRelative[0] >= 1 || screenPositionRelative[0] == 0 || screenPositionRelative[1] >= 1 || screenPositionRelative[1] == 0) {
            return false;
        } else if (screenPositionRelative[2] < 0) {
            return false;
        }
        return true;
    }
}