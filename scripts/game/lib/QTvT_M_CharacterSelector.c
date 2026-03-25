modded class PS_CharacterSelector
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