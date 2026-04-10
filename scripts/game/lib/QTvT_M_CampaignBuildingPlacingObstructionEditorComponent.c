modded class SCR_CampaignBuildingPlacingObstructionEditorComponent : SCR_BaseEditorComponent
{
    override bool IsPreviewOutOfRange(SCR_EditorPreviewParams instantPlacingParam, out ENotification outNotification = -1)
    {
        // In case of WP placed with controller, the preview of the WP doesn't exist and we need to use placing params.
        if (instantPlacingParam)
        {
            vector outTransform[4];
            instantPlacingParam.GetWorldTransform(outTransform);

            if ((vector.DistanceSqXZ(m_AreaTrigger.GetOrigin(), outTransform[3]) >= m_AreaTrigger.GetSphereRadius() * m_AreaTrigger.GetSphereRadius()))
            {
                outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE;
                return true;
            }
        }

        if (!m_AreaTrigger || !m_PreviewEnt)
            return false;

        if ((vector.DistanceSqXZ(m_AreaTrigger.GetOrigin(), m_PreviewEnt.GetOrigin()) >= m_AreaTrigger.GetSphereRadius() * m_AreaTrigger.GetSphereRadius()))
        {
            outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE;
            return true;
        }

        if (vector.DistanceSqXZ(m_AreaTrigger.GetOrigin(), m_PreviewEnt.GetOrigin()) <= 20 * 20)
        {
            outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE;
            return true;
        }

        return false;
    }
}