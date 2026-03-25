enum PS_ETimerCommand
{
	STOP,
	START,
	ADD,
	MINUS
};

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
	
	protected bool m_bTimerEnabled = true;
	
	int GetStepTime()
	{
		return m_iStepTime;
	}
	
	override void OnGameStart()
	{
		super.OnGameStart();
		GetGame().GetCallqueue().CallLater(AddCommands, 0, false);
		
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
		}
		
		m_iStepTime = m_iPreviewTime;
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!Replication.IsServer())
			return;
		
		if (m_bTimerEnabled && m_iStepTime > 0)
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
				m_iStepTime = m_iGameTime + m_iFreezeTime;
				break;
			case SCR_EGameModeState.DEBRIEFING:
				m_iStepTime = m_iDebriefingTime;
				break;
			case SCR_EGameModeState.POSTGAME:
				break;
		}
	}
	
	void AddCommands()
	{
		SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
		ChatCommandInvoker invoker = chatPanelManager.GetCommandInvoker("timer");
		invoker.Insert(SendQTvT_Timer_CommandCallback);
	}
	
	void SendQTvT_Timer_CommandCallback(SCR_ChatPanel panel, string data)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		PS_PlayableControllerComponent playableController = 
			PS_PlayableControllerComponent.Cast(playerController.FindComponent(PS_PlayableControllerComponent));
		if (!playableController)
			return;
		
		if (!PS_PlayersHelper.IsAdminOrServer())
			return;
		
		data = data.Trim();
		string dataLower = data;
		dataLower.ToLower();
		
		string subCommand;
		int timeValue = 0;
		
		int spaceIndex = dataLower.IndexOf(" ");
		if (spaceIndex > -1)
		{
			subCommand = dataLower.Substring(0, spaceIndex);
			string valueStr = dataLower.Substring(spaceIndex + 1, dataLower.Length() - spaceIndex - 1);
			valueStr = valueStr.Trim();
			timeValue = valueStr.ToInt();
		}
		else
		{
			subCommand = dataLower;
		}
		
		switch (subCommand)
		{
			case "stop":
				playableController.SendQTvTTimerCommand(PS_ETimerCommand.STOP, 0);
				break;
			case "start":
				playableController.SendQTvTTimerCommand(PS_ETimerCommand.START, 0);
				break;
			case "add":
				if (timeValue > 0)
					playableController.SendQTvTTimerCommand(PS_ETimerCommand.ADD, timeValue);
				break;
			case "minus":
				if (timeValue > 0)
					playableController.SendQTvTTimerCommand(PS_ETimerCommand.MINUS, timeValue);
				break;
		}
	}
	
	void ProcessTimerCommand(PS_ETimerCommand command, int value)
	{
		switch (command)
		{
			case PS_ETimerCommand.STOP:
				m_bTimerEnabled = false;
				break;
			case PS_ETimerCommand.START:
				m_bTimerEnabled = true;
				break;
			case PS_ETimerCommand.ADD:
				m_iStepTime += value * 1000;
				Replication.BumpMe();
				break;
			case PS_ETimerCommand.MINUS:
				m_iStepTime -= value * 1000;
				if (m_iStepTime < 0)
					m_iStepTime = 1;
				Replication.BumpMe();
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
};

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

modded class PS_PlayableControllerComponent
{
	void SendQTvTTimerCommand(PS_ETimerCommand command, int value)
	{
		Rpc(RPC_SendQTvTTimerCommand, command, value);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_SendQTvTTimerCommand(PS_ETimerCommand command, int value)
	{
		PS_GameModeQuickTvT gameMode = PS_GameModeQuickTvT.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.ProcessTimerCommand(command, value);
	}
}