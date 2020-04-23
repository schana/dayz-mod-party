class SchanaPartyNametagsManager
{
    private ref map<string, ref SchanaPartyNametagsMenu> m_SchanaNametags;

    void SchanaPartyNametagsManager()
    {
        m_SchanaNametags = new map<string, ref SchanaPartyNametagsMenu>();
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.Update, 1000, true);
    }

    void ~SchanaPartyNametagsManager()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(this.Update);
    }

    private void Update()
    {
        PlayerBase activePlayer = GetGame().GetPlayer();
        if (activePlayer && activePlayer.GetIdentity())
        {
            string activePlayerId = activePlayer.GetIdentity().GetId();
            string id;
            foreach (Man man : ClientData.m_PlayerBaseList)
            {
                PlayerBase player = PlayerBase.Cast(man);
                if (player && player.GetIdentity())
                {
                    id = player.GetIdentity().GetId();
                    if (!m_SchanaNametags.Get(id) && id != activePlayerId)
                    {
                        m_SchanaNametags[id] = new SchanaPartyNametagsMenu(player);
                    }
                }
            }
        }
    }
}

static ref SchanaPartyNametagsManager g_schanaPartyNametagsManager;
static ref SchanaPartyNametagsManager GetSchanaPartyNametagsManager()
{
    if (!g_Game.IsServer() && !g_schanaPartyNametagsManager)
    {
        g_schanaPartyNametagsManager = new SchanaPartyNametagsManager;
    }
    return g_schanaPartyNametagsManager;
}
