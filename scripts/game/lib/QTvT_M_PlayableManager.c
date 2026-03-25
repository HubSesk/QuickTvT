modded class PS_PlayableManager
{	
	
	// added another event. i dont know much of that stuff sorry
	void StartTimeBriefing()
	{
		m_iStartTimerCounter -= 1;
		Replication.BumpMe();
		OnStartTimerCounterChanged();
		Print("-+-timerbrifign: " + m_iStartTimerCounter);
		if (m_iStartTimerCounter == 0)
		{
			PS_GameModeCoop gameModeCoop = PS_GameModeCoop.Cast(GetGame().GetGameMode());
			gameModeCoop.AdvanceGameState(SCR_EGameModeState.BRIEFING);
			GetGame().GetCallqueue().Remove(StartTimeBriefing);
		}
	}
	
	
	override void SetFactionReady(FactionKey factionKey, int readyValue)
	{
		RPC_SetFactionReady(factionKey, readyValue);
		Rpc(RPC_SetFactionReady, factionKey, readyValue);
		
		if (m_bFactionsReadySended)
			return;
		
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);
		bool allFactionsReady = true;
		foreach (int playerId : players)
		{
			factionKey = GetPlayerFactionKey(playerId);
			if (factionKey == "")
				continue;
			if (m_mFactionReady[factionKey])
				continue;
			allFactionsReady = false;
			break;
		}
		if (allFactionsReady)
		{
			m_bFactionsReadySended = true;
			
			SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
			ChatCommandInvoker invoker = chatPanelManager.GetCommandInvoker("tmsg");
			invoker.Invoke(null, "Factions ready");
			
			SCR_ChatPanelManager.GetInstance().ShowHelpMessage("Factions ready");
			///now we start countdown to stage advance
			
			m_iStartTimerCounter = 3;
			GetGame().GetCallqueue().CallLater(StartTimeBriefing, 1000, true);
			
		}
	}
	
};