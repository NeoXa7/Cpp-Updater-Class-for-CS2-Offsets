namespace Offsets
{
    // buttons;
    inline int dwForceAttack = 0x181ABC0;
    inline int dwForceAttack2 = 0x181AC50;
    inline int dwForceJump = 0x181B0D0;

    // offsets;
    inline int dwEntityList = 0x19CA848; // uintptr_t
    inline int dwLocalPlayerPawn = 0x182FAE8; // uintptr_t
    inline int dwLocalPlayerController = 0x1A1A690; // uintptr_t
    inline int dwViewAngles = 0x1A36960; // uintptr_t
    inline int dwViewMatrix = 0x1A2CAD0; // uintptr_t
    inline int dwSensitivity = 0x1A29128; // uintptr_t
    inline int dwSensitivity_sensitivity = 0x40; // uintptr_t
    inline int dwGameRules = 0x1A28408; // uintptr_t
    inline int dwGameTypes = 0x1A31B0; // uintptr_t
    inline int dwGameTypes_mapName = 0x120; // uintptr_t
    inline int dwPlantedC4 = 0x1A32040; // uintptr_t
    inline int dwGlobalVars = 0x1823CB0; // uintptr_t
    inline int dwWeaponC4 = 0x19CDC40;  // uintptr_t


    // client_dll;   
    inline int m_iTeamNum = 0x3E3; // uint8
    inline int m_iHealth = 0x344; // int32
    inline int m_ArmorValue = 0x2404; // int32
    inline int m_hPlayerPawn = 0x80C; // CHandle<C_CSPlayerPawn>
    inline int m_flFlashBangTime = 0x13F8; // float32
    inline int m_iIDEntIndex = 0x1458; // CEntityIndex
    inline int m_iShotsFired = 0x23E4; // int32
    inline int m_aimPunchAngle = 0x1584; // QAngle
    inline int m_vOldOrigin = 0x1324; // Vector
    inline int m_vecViewOffset = 0xCB0; // CNetworkViewOffsetVector
    inline int m_pGameSceneNode = 0x328; // CGameSceneNode*
    inline int m_bBombPlanted = 0x9A5; // bool
    inline int m_bBombDropped = 0x9A4; // bool
    inline int m_nBombSite = 0xF94; // int32
    inline int m_bIsScoped = 0x23D0; // bool
    inline int m_sSanitizedPlayerName = 0x770; // CUtlString
    inline int m_fFlags = 0x3EC; // uint32
    inline int m_vecAbsVelocity = 0x3F0; // Vector
    inline int m_vecAbsOrigin = 0xD0; // Vector
    inline int m_hOwnerEntity = 0x440; // CHandle<C_BaseEntity>
    inline int m_iPing = 0x740; // uint32
    inline int m_bHasExploded = 0xFC5; // bool
    inline int m_bBeingDefused = 0xFCC; // bool
    inline int m_flDefuseLength = 0xFDC; // float32
    inline int m_bWarmupPeriod = 0x41; // bool
    inline int m_bCTTimeOutActive = 0x4E; // bool
    inline int m_bTerroristTimeOutActive = 0x4D; // bool
    inline int m_pCameraServices = 0x11E0; // (CPlayer_CameraServices*)
    inline int m_iFOV = 0x210; // uint32
    inline int m_bDormant = 0xEF; // bool
}
