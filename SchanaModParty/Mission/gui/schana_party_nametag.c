class SchanaPartyNametagsMenu extends UIScriptedMenu
{
    static string SCHANA_PARTY_NAMETAG_DELETE = "SCHANA_PARTY_NAMETAG_DELETE";

    private Widget m_SchanaPartyNametagRootWidget;
    private TextWidget m_SchanaPartyNametagTextWidget;
    private PlayerBase m_SchanaPartyNametagPlayer;
    private vector m_SchanaPartyPlayerServerPosition = "0 0 0";
    private string m_SchanaPartyPlayerName = "";

    void SchanaPartyNametagsMenu(PlayerBase player)
    {
        m_SchanaPartyNametagRootWidget = GetGame().GetWorkspace().CreateWidgets("SchanaModParty/GUI/Layouts/nametag.layout");
        m_SchanaPartyNametagTextWidget = TextWidget.Cast(m_SchanaPartyNametagRootWidget);
        m_SchanaPartyNametagRootWidget.Show(false);
        m_SchanaPartyNametagPlayer = player;

        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.SchanaUpdate, 16, true);
    }

    void ~SchanaPartyNametagsMenu()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(this.SchanaUpdate);
        if (m_SchanaPartyNametagRootWidget != null)
        {
            m_SchanaPartyNametagRootWidget.Show(false);
            m_SchanaPartyNametagRootWidget.Unlink();
        }
    }

    void SchanaUpdate()
    {
        if (SchanaPartyNametagShouldShow())
        {
            SchanaPartyNametagUpdate();
        }
        else if (m_SchanaPartyNametagRootWidget != null)
        {
            m_SchanaPartyNametagRootWidget.Show(false);
            delete this;
        }
    }

    private bool SchanaPartyNametagShouldShow()
    {
        if (m_SchanaPartyNametagRootWidget == null)
        {
            return false;
        }
        if (!GetGame().GetPlayer())
        {
            return false;
        }
        if (m_SchanaPartyPlayerName == SCHANA_PARTY_NAMETAG_DELETE)
        {
            return false;
        }
        return true;
    }

    private vector SchanaPartyGetPlayerPosition()
    {
        if (m_SchanaPartyNametagPlayer)
        {
            return m_SchanaPartyNametagPlayer.GetPosition();
        }
        else
        {
            return m_SchanaPartyPlayerServerPosition;
        }
    }

    void SchanaPartyUpdatePosition(vector position)
    {
        m_SchanaPartyPlayerServerPosition = position;
    }

    void SchanaPartyUpdatePlayer(PlayerBase player)
    {
        m_SchanaPartyNametagPlayer = player;
    }

    void SchanaPartyUpdateName(string name)
    {
        m_SchanaPartyPlayerName = name;
    }

    private string SchanaPartyGetPlayerName()
    {
        if (m_SchanaPartyNametagPlayer && m_SchanaPartyNametagPlayer.GetIdentity())
        {
            return m_SchanaPartyNametagPlayer.GetIdentity().GetName();
        }
        else
        {
            return m_SchanaPartyPlayerName;
        }
    }

    void SchanaPartyNametagUpdate()
    {
        float x, y, distance;
        vector position = SchanaPartyGetPlayerPosition();
        vector screenPosition = GetGame().GetScreenPos(position + "0 1.3 0");

        x = Math.Round(screenPosition[0]) - 40;
        y = Math.Round(screenPosition[1]);
        m_SchanaPartyNametagRootWidget.SetPos(x, y);

        distance = Math.Round(vector.Distance(position, GetGame().GetPlayer().GetPosition()));
        string distanceString = distance.ToString() + "m";
        if (distance > 1000)
        {
            distanceString = (Math.Round(distance/100) / 10).ToString() + "km";
        }
        string text = SchanaPartyGetPlayerName() + " " + distanceString;
        m_SchanaPartyNametagTextWidget.SetText(text);

        m_SchanaPartyNametagRootWidget.Show(SchanaPartyNametagVisibleOnScreen());
    }

    private bool SchanaPartyNametagVisibleOnScreen()
    {
        vector position = SchanaPartyGetPlayerPosition();
        vector screenPositionRelative = GetGame().GetScreenPosRelative(position);

        if (screenPositionRelative[0] >= 1 || screenPositionRelative[0] == 0 || screenPositionRelative[1] >= 1 || screenPositionRelative[1] == 0)
        {
            return false;
        }
        else if (screenPositionRelative[2] < 0)
        {
            return false;
        }
        return true;
    }

    PlayerBase SchanaPartyNametagGetPlayer()
    {
        return m_SchanaPartyNametagPlayer;
    }
}
