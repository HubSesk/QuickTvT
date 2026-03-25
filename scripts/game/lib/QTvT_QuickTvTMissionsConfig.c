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