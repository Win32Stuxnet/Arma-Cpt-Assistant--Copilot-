[ComponentEditorProps(category: "Killstats", description: "Logs kills & weapons (server)")]
class StatsLoggerComponentClass : ScriptComponentClass {}

class StatsLoggerComponent : ScriptComponent
{
    protected SCR_BaseGameMode m_GM;

    // Called after entity init
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);

        // server only
        if (!GetGame().IsServer()) return;

        m_GM = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
        if (!m_GM) return;

        // subscribe events
        m_GM.GetOnPlayerKilled().Insert(OnPlayerKilled);
        m_GM.GetOnGameModeEnd().Insert(OnGameEnd);
        		string csvPath = string.Format("$logs:stats_%1.csv", m_RoundStamp);
		m_AppendCsv = FileIO.OpenFile(csvPath, FileMode.APPEND);
		if (m_AppendCsv)
		{
			m_AppendCsv.WriteLine("timestamp,victimId,killerId,weapon");
		}

        Print("[Killstats] StatsLoggerComponent initialized (server).", LogLevel.NORMAL);
    }

    override void OnDelete(IEntity owner)
    {
        super.OnDelete(owner);

        if (m_GM)
        {
            m_GM.GetOnPlayerKilled().Remove(OnPlayerKilled);
            m_GM.GetOnGameModeEnd().Remove(OnGameEnd);
        }
        if (m_AppendCsv)
		{
			m_AppendCsv.Close();
			m_AppendCsv = null;
		}
    }

    // --- event handlers ---
    void OnPlayerKilled(int victimId, IEntity victim, IEntity killerEnt, notnull Instigator killer)
    {
        PrintFormat("[Killstats] Victim %1 killed by %2", victimId, killer.GetPlayerID());
    }

    void OnGameEnd()
    {
        Print("[Killstats] Round ended, stats summary here");
    }
}
