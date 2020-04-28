class SchanaPartyMenu extends UIScriptedMenu
{
	private bool m_SchanaPartyIsInitialized;
	private bool m_SchanaPartyMenuIsOpen;

	private TextListboxWidget m_SchanaPartyPlayerList;
	private TextListboxWidget m_SchanaPartyPartyList;
	private ButtonWidget m_SchanaPartyButtonAdd;
	private ButtonWidget m_SchanaPartyButtonRemove;

	private ref map<string, string> member_sorting_map;

	void SchanaPartyMenu()
	{
		member_sorting_map = new map<string, string>();
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.SchanaPartyUpdatePartyStatus, 200, true);
	}

	override Widget Init()
	{
		if (!m_SchanaPartyIsInitialized)
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
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(this.SchanaPartyUpdatePartyStatus);

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

		switch (w)
		{
		case m_SchanaPartyButtonAdd:
			selectedRow = m_SchanaPartyPlayerList.GetSelectedRow();
			if (selectedRow == -1)
			{
				break;
			}
			m_SchanaPartyPlayerList.GetItemData(selectedRow, 0, id);
			Print("[SchanaParty] Menu add " + id.param1);
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
			Print("[SchanaParty] Menu remove " + id.param1);
			GetSchanaPartyManagerClient().RemovePlayerFromParty(id.param1);
			SchanaPartyUpdateLists();
			m_SchanaPartyPartyList.SelectRow(selectedRow - 1);
			break;
		}
		return super.OnClick(w, x, y, button);
	}

	void SchanaPartyUpdateLists()
	{
		if (m_SchanaPartyIsInitialized)
		{
			SchanaPartyUpdatePartyList();
			SchanaPartyUpdatePlayerList();
			SchanaPartyUpdatePartyStatus();
		}
	}

	void SchanaPartyUpdatePartyList()
	{
		member_sorting_map.Clear();
		string member_name;
		int insert_row = 0;

		auto members = GetSchanaPartySettings().GetMembers();
		foreach (auto id : members)
		{
			member_name = GetSchanaPartySettings().GetName(id);
			member_sorting_map.Insert(member_name + id, id);
		}

		auto sorted_keys = member_sorting_map.GetKeyArray();
		sorted_keys.Sort();
		foreach (auto sorted_key : sorted_keys)
		{
			string member_id = member_sorting_map.Get(sorted_key);
			member_name = GetSchanaPartySettings().GetName(member_id);
			if (insert_row < m_SchanaPartyPartyList.GetNumItems())
			{
				m_SchanaPartyPartyList.SetItem(insert_row, member_name, new Param1<string>(member_id), 0);
			}
			else
			{
				m_SchanaPartyPartyList.AddItem(member_name, new Param1<string>(member_id), 0);
			}
			insert_row++;
		}

		while (insert_row < m_SchanaPartyPartyList.GetNumItems())
		{
			m_SchanaPartyPartyList.SelectRow(-1);
			m_SchanaPartyPartyList.RemoveRow(insert_row);
		}

		if (m_SchanaPartyPartyList.GetSelectedRow() >= insert_row)
		{
			m_SchanaPartyPartyList.SelectRow(insert_row - 1);
		}
	}

	void SchanaPartyUpdatePlayerList()
	{
		member_sorting_map.Clear();
		int insert_row = 0;

		auto onlinePlayers = GetSchanaPartyManagerClient().GetOnlinePlayers();
		foreach (auto id, auto player_name : onlinePlayers)
		{
			member_sorting_map.Insert(player_name + id, id);
		}

		auto sorted_keys = member_sorting_map.GetKeyArray();
		sorted_keys.Sort();
		foreach (auto sorted_key : sorted_keys)
		{
			string player_id = member_sorting_map.Get(sorted_key);

			if (insert_row < m_SchanaPartyPlayerList.GetNumItems())
			{
				m_SchanaPartyPlayerList.SetItem(insert_row, onlinePlayers.Get(player_id), new Param1<string>(player_id), 0);
			}
			else
			{
				m_SchanaPartyPlayerList.AddItem(onlinePlayers.Get(player_id), new Param1<string>(player_id), 0);
			}
			insert_row++;
		}

		while (insert_row < m_SchanaPartyPlayerList.GetNumItems())
		{
			m_SchanaPartyPlayerList.RemoveRow(insert_row);
		}

		if (m_SchanaPartyPlayerList.GetSelectedRow() >= insert_row)
		{
			m_SchanaPartyPlayerList.SelectRow(insert_row - 1);
		}
	}

	void SchanaPartyUpdatePartyStatus()
	{
		Param1<string> id;
		for (int i = 0; i < m_SchanaPartyPartyList.GetNumItems(); ++i)
		{
			m_SchanaPartyPartyList.GetItemData(i, 0, id);

			if (GetSchanaPartyManagerClient().IsPartyMemberOnline(id.param1))
			{
				// Green A400
				m_SchanaPartyPartyList.SetItemColor(i, 0, 0xFF00E676);
			}
			else if (GetSchanaPartyManagerClient().IsPartyMemberOnlineButNotMutual(id.param1))
			{
				// Light Blue A400
				m_SchanaPartyPartyList.SetItemColor(i, 0, 0xFF00B0FF);
			}
			else
			{
				// Gray 400
				m_SchanaPartyPartyList.SetItemColor(i, 0, 0xFFBDBDBD);
			}
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
