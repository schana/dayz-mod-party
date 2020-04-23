class SchanaPartyNametagsMenu extends UIScriptedMenu
{
    private Widget m_SchanaPartyNametagRootWidget;
    private TextWidget m_SchanaPartyNametagTextWidget;
    private PlayerBase m_SchanaPartyNametagPlayer;

    void SchanaPartyNametagsMenu(PlayerBase player)
    {
        m_SchanaPartyNametagRootWidget = GetGame().GetWorkspace().CreateWidgets("SchanaModParty/GUI/Layouts/nametag.layout");
        m_SchanaPartyNametagTextWidget = TextWidget.Cast(m_SchanaPartyNametagRootWidget);
        m_SchanaPartyNametagRootWidget.Show(false);
        m_SchanaPartyNametagPlayer = player;

        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.SchanaUpdate, 10, true);
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
        if (!m_SchanaPartyNametagPlayer)
        {
            return false;
        }
        if (!m_SchanaPartyNametagPlayer.GetIdentity())
        {
            return false;
        }
        return true;
    }

    void SchanaPartyNametagUpdate()
    {
        float x, y, distance;
        vector position = m_SchanaPartyNametagPlayer.GetPosition();

        vector screenPosition = GetGame().GetScreenPos(position + "0 1.3 0");
        x = Math.Round(screenPosition[0]) - 40;
        y = Math.Round(screenPosition[1]);
        distance = Math.Round(screenPosition[2]);
        m_SchanaPartyNametagRootWidget.SetPos(x, y);
        string text = m_SchanaPartyNametagPlayer.GetIdentity().GetName() + " " + distance.ToString() + "m";
        m_SchanaPartyNametagTextWidget.SetText(text);
        m_SchanaPartyNametagRootWidget.Show(SchanaPartyNametagVisibleOnScreen());
    }

    private bool SchanaPartyNametagVisibleOnScreen()
    {
        vector position = m_SchanaPartyNametagPlayer.GetPosition();
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
