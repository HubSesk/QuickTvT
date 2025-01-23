class PS_GameModeQuickTvTClass: PS_GameModeCoopClass
{
};

class PS_GameModeQuickTvT : PS_GameModeCoop
{
	static const string m_QuickTvTConfigFilePath = "$profile:PS_QuickTvT_Config.json";
	protected static ref PS_QuickTvTMissionsConfig m_QuickTvTMissionsConfig;
	
	[Attribute("15000", UIWidgets.EditBox, "", "", category: "Reforger Lobby")]
	int m_iPreviewTime;
	
	[Attribute("30000", UIWidgets.EditBox, "", "", category: "Reforger Lobby")]
	int m_iSlotsTime;
	
	[Attribute("30000", UIWidgets.EditBox, "", "", category: "Reforger Lobby")]
	int m_iBriefingTime;
	
	[Attribute("480000", UIWidgets.EditBox, "", "", category: "Reforger Lobby")]
	int m_iGameTime;
	
	[Attribute("12000", UIWidgets.EditBox, "", "", category: "Reforger Lobby")]
	int m_iDebriefingTime;
	
	[RplProp()]
	protected int m_iStepTime;
	
	protected static int m_iMissionNum = 0;
	
	protected PlayerManager m_PlayerManager;
	
	int GetStepTime()
	{
		return m_iStepTime;
	}
	
	override void OnGameStart()
	{
		super.OnGameStart();
		
		if (!m_QuickTvTMissionsConfig)
		{
			m_QuickTvTMissionsConfig = new PS_QuickTvTMissionsConfig();
			SCR_JsonLoadContext configLoadContext = new SCR_JsonLoadContext();
			configLoadContext.LoadFromFile(m_QuickTvTConfigFilePath);
			configLoadContext.ReadValue("", m_QuickTvTMissionsConfig);
			
			m_QuickTvTMissionsConfig.SortRandom();
		}
		
		m_PlayerManager = GetGame().GetPlayerManager();
		
		if (m_iGameTime == 0)
		{
			m_iMissionNum = 0;
			ChangeToNextMission();
		}
		
		m_iStepTime = m_iPreviewTime;
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!Replication.IsServer())
			return;
		
		if (m_iStepTime > 0)
		{
			int playersCount = m_PlayerManager.GetPlayerCount();
			if (GetState() != SCR_EGameModeState.PREGAME || playersCount > 1)
				m_iStepTime -= timeSlice * 1000;
			
			if (m_iStepTime <= 0)
				AdvanceGameState(GetState());
			
			Replication.BumpMe();
		}
	}
	
	override void OnGameStateChanged()
	{
		super.OnGameStateChanged();
		
		SCR_EGameModeState state = GetState();
		switch (state) 
		{
			case SCR_EGameModeState.PREGAME:
				m_iStepTime = m_iPreviewTime;
				break;
			case SCR_EGameModeState.SLOTSELECTION:
				m_iStepTime = m_iSlotsTime;
				break;
			case SCR_EGameModeState.CUTSCENE:
				break;
			case SCR_EGameModeState.BRIEFING:
				m_iStepTime = m_iBriefingTime;
				break;
			case SCR_EGameModeState.GAME:
				if (Replication.IsServer())
					GetGame().GetCallqueue().CallLater(CheckAlive, 3000, true);
				m_iStepTime = m_iGameTime;
				break;
			case SCR_EGameModeState.DEBRIEFING:
				GetGame().GetCallqueue().Remove(CheckAlive);
				m_iStepTime = m_iDebriefingTime;
				break;
			case SCR_EGameModeState.POSTGAME:
				ChangeToNextMission();
				break;
		}
	}
	
	void CheckAlive()
	{
		PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();
		
		FactionKey checkFaction = "";
		array<PS_PlayableContainer> playables = playableManager.GetPlayablesSorted();
		foreach (PS_PlayableContainer playable : playables)
		{
			EDamageState damageState = playable.GetDamageState();
			if (damageState == EDamageState.DESTROYED)
				continue;
			
			FactionKey factionKey = playable.GetFactionKey();
			if (checkFaction != "" && checkFaction != factionKey)
			{
				return;
			}
			checkFaction = factionKey;
		}
		
		m_OnOnlyOneFactionAlive.Invoke(checkFaction);
		AdvanceGameState(SCR_EGameModeState.GAME);
		GetGame().GetCallqueue().Remove(CheckAlive);
	}
	
	void ChangeToNextMission()
	{
		int playersCount = m_PlayerManager.GetPlayerCount();
		
		// Get next mission
		m_iMissionNum++;
		if (m_iMissionNum >= m_QuickTvTMissionsConfig.Missions.Count())
		{
			m_iMissionNum = 0;
			m_QuickTvTMissionsConfig.SortRandom();
		}
		PS_QuickTvTMission mission = m_QuickTvTMissionsConfig.Missions[m_iMissionNum];
		
		// skip to next valide
		int i = 0;
		while (mission.MinPlayers > playersCount || mission.MaxPlayers < playersCount)
		{
			i++;
			if (i > 100) break;
			
			m_iMissionNum++;
			if (m_iMissionNum >= m_QuickTvTMissionsConfig.Missions.Count())
			{
				m_iMissionNum = 0;
				m_QuickTvTMissionsConfig.SortRandom();
			}
			
			mission = m_QuickTvTMissionsConfig.Missions[m_iMissionNum];
		}
		GameStateTransitions.RequestScenarioChangeTransition(mission.MissionConfig, "");
	}
	
	// 
	// ------==========  partyzans great code start  =================----------
	//
	
	// this changes balance script to allow players join faction at same ratio to avaivable human players as it has to overall playable units
	override bool CanJoinFaction(FactionKey factionKeyPlayer, FactionKey currentFaction)
	{
		if (m_iFactionsBalance == -1)
			return true;
		if (factionKeyPlayer == currentFaction)
			return true;
		
		map<FactionKey, int> players = new map<FactionKey, int>();
		map<FactionKey, int> playables = new map<FactionKey, int>();
		map<FactionKey, float> desiredratio = new map<FactionKey, float>();
		array<PS_PlayableContainer> playableComponents = m_playableManager.GetPlayablesSorted();
		
		//counting avaivable slots
		int playablesammount = 0;
		foreach (PS_PlayableContainer playable : playableComponents)
		{
			playablesammount = playablesammount + 1;
			FactionKey factionKey = playable.GetFactionKey();
			if (!players.Contains(factionKey))
				players[factionKey] = 0;
			if (!playables.Contains(factionKey))
				playables[factionKey] = 0;
			
			playables[factionKey] = playables[factionKey] + 1;
			int playerId = m_playableManager.GetPlayerByPlayable(playable.GetRplId());
			if (playerId > 0)
			{
				players[factionKey] = players[factionKey] + 1;
			}
			
		}
		if (currentFaction != "")
			players[currentFaction] = players[currentFaction] - 1;
		
		//counting how much there are factons units compared to every unit
		int playersCount = m_PlayerManager.GetPlayerCount();
		foreach (FactionKey factionKey, int count: playables)
		{	
			desiredratio[factionKey] = count / playablesammount; 
		}
		//clamping avaivable over the ratio slots in proportion to current player count to ensure balance for small scenarios
		int adjfactionsbalance = Math.Clamp(m_iFactionsBalance,1,(playersCount / 10));
		
		if (players[factionKeyPlayer] < 1)
		{
			//but we still want to get one player even in tiniest scenario
			return true;
		}
		//check if that faction with a new player wouldnt get too many players
		float ratio = (players[factionKeyPlayer] + 1 - adjfactionsbalance) / playersCount;
		
		return ratio <= desiredratio[factionKeyPlayer];
	}	
	
};



// partyzan minor edit. Ensure we have message which doesent rely on another addon. You can scrap that if you dont like that
modded class PS_CharacterSelector : SCR_ButtonComponent
{
	override void OnClicked(SCR_ButtonBaseComponent button)
	{
		if (m_bStateClickSkip)
		{
			m_bStateClickSkip = false;
			return;
		}
		
		int playerId = m_CoopLobby.GetSelectedPlayer();
		if (m_iPlayerId == -2)
		{
			m_CoopLobby.SetPreviewPlayable(m_iPlayableId, true);
			AudioSystem.PlaySound("{C97850E4341F0CF9}Sounds/UI/Samples/Menu/UI_Button_Fail.wav");
			return;
		}
		if (m_iPlayerId > 0 && playerId != m_iPlayerId)
		{
			m_CoopLobby.SetPreviewPlayable(m_iPlayableId, true);
			AudioSystem.PlaySound("{C97850E4341F0CF9}Sounds/UI/Samples/Menu/UI_Button_Fail.wav");
			return;
		}
		PS_PlayableContainer playableContainer = m_PlayableManager.GetPlayableById(m_iPlayableId);
		if (playableContainer.GetDamageState() == EDamageState.DESTROYED)
		{
			m_CoopLobby.SetPreviewPlayable(m_iPlayableId, true);
			AudioSystem.PlaySound("{C97850E4341F0CF9}Sounds/UI/Samples/Menu/UI_Button_Fail.wav");
			return;
		}
	
		SCR_EGameModeState gameState = m_GameModeCoop.GetState();
		if (!PS_PlayersHelper.IsAdminOrServer())
		{
			RplId playableId = m_PlayableManager.GetPlayableByPlayer(m_iCurrentPlayerId);
			if (gameState == SCR_EGameModeState.BRIEFING && playableId != RplId.Invalid())
			{
				m_CoopLobby.SetPreviewPlayable(m_iPlayableId, true);
				return;
			}
		}
		
		
		
		if (playerId != m_iPlayerId)
		{
			if (!CanJoinFaction())
			{
				
				SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
				ChatCommandInvoker invoker = chatPanelManager.GetCommandInvoker("lmsg");
				invoker.Invoke(null, "Где баланс?");
				SCR_ChatPanelManager.GetInstance().ShowHelpMessage("Соблюдайте баланс сторон");
				m_CoopLobby.SetPreviewPlayable(m_iPlayableId, true);
				return;
			}
			
			AudioSystem.PlaySound("{9500A96BBA3B0581}Sounds/UI/Samples/Menu/UI_Gadget_Select.wav");
			m_PlayableControllerComponent.MoveToVoNRoom(playerId, m_sFactionKey, m_sPlayableCallsign);
			m_PlayableControllerComponent.ChangeFactionKey(playerId, m_sFactionKey);
			m_PlayableControllerComponent.SetPlayerState(playerId, PS_EPlayableControllerState.NotReady);	
			m_PlayableControllerComponent.SetPlayerPlayable(playerId, m_iPlayableId);
		} else {
			AudioSystem.PlaySound("{9500A96BBA3B0581}Sounds/UI/Samples/Menu/UI_Gadget_Select.wav");
			m_PlayableControllerComponent.MoveToVoNRoom(playerId, m_sFactionKey, "#PS-VoNRoom_Faction");
			m_PlayableControllerComponent.ChangeFactionKey(playerId, "");
			m_PlayableControllerComponent.SetPlayerState(playerId, PS_EPlayableControllerState.NotReady);
			m_PlayableControllerComponent.SetPlayerPlayable(playerId, RplId.Invalid());
			if (PS_PlayersHelper.IsAdminOrServer())
				m_PlayableControllerComponent.UnpinPlayer(playerId);
		}
		
		if (PS_PlayersHelper.IsAdminOrServer() && playerId != m_iCurrentPlayerId && gameState == SCR_EGameModeState.GAME)
			m_PlayableControllerComponent.ForceSwitch(playerId);
		if (!PS_PlayersHelper.IsAdminOrServer() && playerId == m_iCurrentPlayerId && gameState == SCR_EGameModeState.BRIEFING)
			m_PlayableControllerComponent.SwitchToMenuServer(SCR_EGameModeState.BRIEFING);
	}
	
};


// partyzan minor edit. there i start game on both factions declare ready while briefing
modded class PS_PlayableManager : ScriptComponent
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




// 
// ------==========  partyzans great code end  =================----------
//



class PS_QuickTvTMissionsConfig: JsonApiStruct
{
	ref array<ref PS_QuickTvTMission> Missions = {};
	ref RandomGenerator m_RandomGenerator = new RandomGenerator();
	
	void SortRandom()
	{
		array<ref PS_QuickTvTMission> MissionsNew = {};
		foreach (PS_QuickTvTMission mission : Missions)
		{
			MissionsNew.InsertAt(mission, m_RandomGenerator.RandInt(0, MissionsNew.Count()));
		}
		Missions = MissionsNew;
	}
	
	void PS_QuickTvTMissionsConfig()
	{
		RegV("Missions");
	}
}


class PS_QuickTvTMission: JsonApiStruct
{
	string MissionConfig;
	int MinPlayers;
	int MaxPlayers;
	
	void PS_QuickTvTMission()
	{
		RegV("MissionConfig");
		RegV("MinPlayers");
		RegV("MaxPlayers");
	}
}





















