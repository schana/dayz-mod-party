class SchanaPartyMenu extends UIScriptedMenu
{
    private bool m_SchanaPartyIsInitialized;
    private bool m_SchanaPartyMenuIsOpen;

    private TextListboxWidget m_SchanaPartyPlayerList;
    private TextListboxWidget m_SchanaPartyPartyList;
    private ButtonWidget m_SchanaPartyButtonAdd;
    private ButtonWidget m_SchanaPartyButtonRemove;

    override Widget Init()
    {
        if(!m_SchanaPartyIsInitialized)
        {
            layoutRoot = GetGame().GetWorkspace().CreateWidgets("SchanaModParty/GUI/Layouts/manager.layout");
            m_SchanaPartyPlayerList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("Players"));
            m_SchanaPartyPartyList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("Party"));
            m_SchanaPartyButtonAdd = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ButtonAdd"));
            m_SchanaPartyButtonRemove = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ButtonRemove"));

            m_SchanaPartyIsInitialized = true;
        }

        SchanaPartyUpdateLists();
        
        return layoutRoot;
    }

    void ~SchanaPartyMenu()
    {
        GetGame().GetUIManager().Back();
        g_Game.GetUIManager().ShowCursor(true);
        g_Game.GetUIManager().ShowUICursor(false);
        GetGame().GetInput().ResetGameFocus();
        GetGame().GetMission().PlayerControlEnable(false);
        GetGame().GetMission().GetHud().Show(true);

        if (layoutRoot)
        {
            layoutRoot.Unlink();
        }
    }

    override void OnHide()
    {
        super.OnHide();

        g_Game.GetUIManager().ShowCursor(true);
        g_Game.GetUIManager().ShowUICursor(false);
        GetGame().GetInput().ResetGameFocus();
        GetGame().GetMission().PlayerControlEnable(false);
        GetGame().GetMission().GetHud().Show(true);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
		int selectedRow;
		Param1<string> id;
		
    	switch(w)
    	{
    		case m_SchanaPartyButtonAdd:
			selectedRow = m_SchanaPartyPlayerList.GetSelectedRow();
			if (selectedRow == -1)
			{
				break;
			}
			m_SchanaPartyPlayerList.GetItemData(selectedRow, 0, id);
			Print("[SchanaParty] add " + id.param1);
            GetSchanaPartyManagerClient().AddPlayerToParty(id.param1);
			SchanaPartyUpdateLists();
            break;

            case m_SchanaPartyButtonRemove:
			selectedRow = m_SchanaPartyPartyList.GetSelectedRow();
			if (selectedRow == -1)
			{
				break;
			}
			m_SchanaPartyPartyList.GetItemData(selectedRow, 0, id);
			Print("[SchanaParty] remove " + id.param1);
            GetSchanaPartyManagerClient().RemovePlayerFromParty(id.param1);
			SchanaPartyUpdateLists();
            break;
    	}
        return super.OnClick(w, x, y, button);
    }

    void SchanaPartyUpdateLists()
    {
        m_SchanaPartyPartyList.ClearItems();
        auto members = SchanaModPartySettings.Get().GetMembers();
        foreach(auto member_id : members)
        {
            int row = m_SchanaPartyPartyList.AddItem(SchanaModPartySettings.Get().GetName(member_id), new Param1<string>(member_id), 0);
			if (GetSchanaPartyManagerClient().IsPartyMemberOnline(member_id))
			{
				m_SchanaPartyPartyList.SetItemColor(row, 0, 0xFF00E676);
			}
        }
		
		m_SchanaPartyPlayerList.ClearItems();
		auto onlinePlayers = GetSchanaPartyManagerClient().GetOnlinePlayers();
		foreach(auto player_id, auto player_name : onlinePlayers)
		{
			m_SchanaPartyPlayerList.AddItem(player_name, new Param1<string>(player_id), 0);
		}
    }

    bool SchanaPartyMenuIsOpen()
    {
        return m_SchanaPartyMenuIsOpen;
    }

    void SchanaPartyMenuSetOpen(bool open)
    {
        m_SchanaPartyMenuIsOpen = open;
    }
}
