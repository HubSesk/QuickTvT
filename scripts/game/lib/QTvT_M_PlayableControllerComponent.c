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