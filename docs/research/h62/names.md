# h62 -- the name map

*2026-10-11 22:39Z. Part of the h62 set ([README](README.md)): Harry62's findings, credited to him and restated in
our words. Every h62 claim is believed until a read or probe of ours stands behind it; KNOWN wins.*

## His labels, counted

| | count |
|---|---|
| labels he wrote by hand (the tool's own 63,099 automatic string markers excluded) | 5,956 |
| -- identifiers | 3,004 |
| -- mangled C++ names | 624 |
| -- free-text notes on instructions and data ("load weapon ID", "packet size") | 1,914 |
| -- bare numbers (his numbered cross-references between notes) | 414 |
| on an r0004 function start (`recomp/socom2_ghidra_r0004.csv`, 16,417 functions) | 1,334 |
| inside a function | 4,047 |
| outside every function (data) | 575 |

The data count above uses the function map as it stands; it treats a few over-long map rows as functions. Counted
against sane function bodies only, 2,451 labels sit on data; 1,577 of those are SOCOM 1 demo symbols at the demo's
addresses ([caveats.md](caveats.md)), and of the other 874, 265 are referenced by r0004 code.

## Where his names meet ours: 524 agree, 55 differ

Wherever we already name an r0004 function start (`recomp/socom2_names_r0004.csv`, 1,736 rows) and he has a label
there, the two were compared ignoring case and punctuation, accepting our short name or our mangled form, and
accepting his label when it begins with our name. **524 agree; 55 differ.** None of the 524 agreements is one of his
demo-address imports (0 of 524), so each is his placement agreeing with ours. The overlap leans to the library floor
(RT, libnetb, sce, libc), which corroborates our `exact` and `toml-stub` rows more than our inferred game-code names.

The 55 differences, sorted:

| kind | count | meaning |
|---|---|---|
| his note appended to our very name | 3 | agreement: `CZNetwork_zNetSendApplicationMessage` `0x0032a420` ("packet sender 1"), `CCharacterWeap_SetupCharacterWeapon` `0x00543930` ("init weapon selected"), `CZSealBody_RemoteRespawn` `0x0059e690` ("respawn player") |
| his short alias for our full name | 18 | same function, fewer letters; ours stand (table below) |
| his purpose note where we have an identity | 6 | a cheat-maker names what patching the body does; ours stand |
| a free-text note on a start we name | 23 | a description, not a name; mostly agreeing in substance (table below) |
| **real conflicts** | **5** | name against name; each wants one body read (below) |

### The 18 aliases

| r0004 | his | ours (our pass) |
|---|---|---|
| `0x001b4888` | MCCHANGEDIR | `sceMcChdir` (toml-stub) |
| `0x00247d30` | libnetb_recv_from | `netbExUdpRecv` (hand) |
| `0x00247fe8` | libnetb_send_to | `netbExUdpSend` (hand) |
| `0x00279d10` | BanSelfFromGame | `UIBanSelfFromGame` (ui-binding) |
| `0x0027a860` | GetUsagePolicy | `UIGetUsagePolicy` (ui-binding-derived) |
| `0x0027adf0` | GetGameInfo | `UIGetGameInfo` (ui-binding-derived) |
| `0x0027b800` | InvitePlayerToClan | `UIInvitePlayerToClan` (string-set and ui-binding) |
| `0x0027e480` | joinmproom | `UIJoinMPRoom` (ui-binding) |
| `0x0027e9e0` | CreateMPGame | `UICreateMPGame` (ui-binding) |
| `0x0027ea40` | InitArmory | `UIInitializeOnlineArmory` (ui-binding-derived) |
| `0x0027f9f0` | IsSessionMaster | `UIIsSessionMaster` (ui-binding) |
| `0x00282070` | uiweapon | `UICanCharactersSelectUIWeapon` (ui-binding-derived) |
| `0x00283af0` | ChoosePrimary | `UIChoosePrimary` (ui-binding-derived) |
| `0x0061df60` | NetIInitialize | `NetInitialize` (string-set) |
| `0x006315f8` | md5Init | `sceEENetMD5Init` (toml-stub) |
| `0x006456a0` | rt_time_get_time_string | `rt_time_get_time_string_ptr` (string-set) |
| `0x0063ef08` | rt_msg_client_process_anti_cheat | `serverMemRead` (hand) |
| `0x0063f788` | rt_msg_client_process_memory_poke | `serverMemWrite` (hand) |

The last two are the same reading of the same two bodies from the other side: the server's anti-cheat reads and
writes client memory through them ([network.md](network.md)).

### The six purpose notes

| r0004 | his | ours (our pass) | what his name tells us |
|---|---|---|---|
| `0x002785c0` | UpdatePlayerStats | `UISetPlayerDescription` (ui-binding) | the description field rides the stats update |
| `0x002fae30` | shootthroughobjects | `CMaterialExArray_ReadMaterials` (string-set) | material penetration values are read here |
| `0x00305b70` | PlayerInfo | `cbGetMyClans` (string-set) | the clan callback fills the player's info |
| `0x003f93c8` | CreateGameOnSelfRequest | `MediusCheckRequestQueue` (callgraph) | a game-creation request passes this queue |
| `0x0054c3c0` | MessageFeed_PrintKill | `CZSealBody_OnDeath` (string-set and callgraph) | the kill line in the message feed is printed from the death handler |
| `0x005c3cb0` | perfectshot | `CZKit_SetCurAccuracy` (callgraph) | the accuracy (bloom) setter; see the PlayerObject bloom fields |

### The 23 notes on starts we name

| r0004 | his note | ours | reads as |
|---|---|---|---|
| `0x00193628` | tlb miss | `_free_r` | differs; ours (toml-stub and callgraph) stands |
| `0x001a74f0` | fnc_WRITE PATCH 1 | `_sceFs_Rcv_Intr` | his patch's hook site, not a name |
| `0x001b3d40` / `0x001b3ea0` / `0x001b40c0` | MC OPEN / MC CLOSE / MC READ? | `sceMcOpen` / `sceMcClose` / `sceMcRead` | agrees |
| `0x001b41d8` | FILE PTR | `sceMcWrite` | a note on its argument |
| `0x001bd790` | pad read | `scePad2Read` | agrees |
| `0x002474f8` | packet sender? | `netbExTcpRecv` | differs; ours is the runtime's hooked receive |
| `0x0027e940` | join mp game | `UIJoinMPGame` | agrees |
| `0x002839d0` | find weapon parent1 | `UIChooseSecondary` | a note on its caller chain |
| `0x002b01d0` | start event | `HUDTriggerMissionCamBegin` | agrees in substance |
| `0x002bd790` | push blend packet sender | `NetSendPushBlend` | agrees |
| `0x002bfdb0` | update stats? | `CZSealState_IncrementalUpdateRemoteSeal` | partly: the remote seal update carries stats fields |
| `0x002cf330` | DNAS scan | `UIDNASAuthenticate` (the runtime's `dnasCheck`) | agrees |
| `0x002f4060` | (our name, his note "maprender") | `zdb_CGrid_addOrderedCellAtom` | agrees, with a use note |
| `0x00312100` | whisper receiver | `chatFanoutRecv` | agrees, and says which chat |
| `0x0033e9a0` | LoadTexture? | `zdb_CAssetList_GetTexHandle` | agrees in substance |
| `0x00355f50` | render texture fnc1 | `node2` (the runtime's address table) | agrees in substance |
| `0x003a65b0` | string color fnc? | `CUIVariable_op_assign` | differs; ours (callgraph) is inferred, his is a question |
| `0x003d2080` | fps fnc | `zVid_Swap` | agrees: the per-frame buffer swap |
| `0x0054c0d0` | PickupWeapon parent | `CZSealBody_PickupObject` | agrees |
| `0x005a15a0` | player dynamics | `CharacterDynamics_Load` | agrees |
| `0x0061ccd0` | packet sender broadcast | `NetSendMessageByUDPToIP` | agrees |

### The five real conflicts

| r0004 | r0001 twin (how) | his | ours (pass, score) | reading |
|---|---|---|---|---|
| `0x0027b310` | `0x0027a250` exact | `UIGameStatePopCommand` (mangled, a demo symbol) | `UIGetClanRanking` (ui-binding-derived, 0.80: from the command string `GetClanRanking`, no demo counterpart) | the body is 40 bytes, the size of the demo's `UIGameStatePopCommand`; ours comes from the binding table's string. The strongest case against one of ours; one body read |
| `0x0027ba30` | `0x0027aa60` seed+delta | `UIIsMemberOfClan` (mangled, a demo symbol) | `UIGetClan` (ui-binding, 0.90) | a 12-byte body; the demo's `UIIsMemberOfClan` and `UIGetClan` are both 12 bytes, so size cannot choose; ours has the binding row |
| `0x00184f20` | `0x00184f20` seed+delta | a `std::vector_deleter<node>` destructor | `std_basic_filebuf_dtor_6989d1` (vtable-slot, 0.75) | his mangled name is not in the demo's symbol table; ours is from a vtable slot. One read of what the destructor frees |
| `0x003045a0` | `0x002e7600` relinked-body | SendClanInvite | `CZOnlineLobby_InvitePlayerToClan` (string-set, 0.80) | probably the same function in his words; ours has the demo's identity |
| `0x002a0b10` | `0x0029f610` relinked-body | ClearPlayerList | `CEntity_RemoveAll` (offset-multiset, 0.75) | ours is inferred; his says what the body does in the online game. Both may be right |

Until a body read, ours stand in the sidecar; his enter as aliases. A wrong one of ours is silent (the runtime binds
by address: research/61), so these reads are cheap insurance, not a defect hunt.

## The 512 names new to us

Where he names an r0004 function start that our sidecar leaves unnamed (`FUN_`), the name is new to us: **512**,
none applied since (re-checked 2026-10-11: still 0 in `recomp/socom2_names_r0004.csv`). 62 sit in the shared loader
region (`0x100000`-`0x1d5600`), 450 in the overlays.

**By mapping class** (the r0001 twin through `game/r0004/match.json`, read the other way):

| class | count | what it means for the name |
|---|---|---|
| exact | 299 | carries to r0001 byte for byte; 15 of the r0001 twins already carry a name of ours, all agreeing |
| relinked-body | 80 | same body, relocations differ; carries with the matcher's confidence |
| seed+delta | 31 | placed by distance from a seed; a pointer to where to look, not a match |
| hash+callees | 1 | |
| none | 101 | **r0004-only**: no r0001 function corresponds; these names can live only in an r0004 map |

**By category** (our sort, by name and region):

| category | count | exact | relinked | seed+delta | hash | none |
|---|---|---|---|---|---|---|
| game classes, mangled (Zipper's own names) | 130 | 102 | 14 | 2 | 0 | 12 |
| network: transport, RT, Medius | 73 | 31 | 17 | 9 | 0 | 16 |
| SDK, libc and middleware | 72 | 42 | 1 | 1 | 0 | 28 |
| game online and session (his readings and mangled) | 59 | 20 | 16 | 3 | 0 | 20 |
| voice and speech recognition | 56 | 47 | 2 | 5 | 0 | 2 |
| his purpose notes and scratch labels | 51 | 17 | 21 | 1 | 0 | 12 |
| render, HUD and menus, mangled | 37 | 29 | 1 | 5 | 0 | 2 |
| game logic, his readings | 34 | 11 | 8 | 5 | 1 | 9 |
| **total** | **512** | **299** | **80** | **31** | **1** | **101** |

187 of the 194 among them that carry a C++ mangling or a compiler-made symbol exist in the SOCOM 1 demo's symbol table: Zipper's identities, placed on SOCOM
II bodies by him. His purpose notes and scratch labels (`health2`, `recoil1`, `infammo`, `unk1`, `d`) mark functions he
found by patching them; they say where to look, not what the function is, and should not enter the sidecar as names
([caveats.md](caveats.md)).

**How they would enter the project.** Not by hand into the sidecar: `tools_py/apply_names.py` is its only writer. A
lever with `Pass`/`Source` provenance (research/89 Q5) proposes them; the `exact` and `relinked-body` ones can carry to
r0001; the `none` ones go to the r0004 sidecar directly; the purpose notes stay aliases. Applying names changes the
generated code, so it rides a recomp chain and the gate (`run-gate`). Names are written below in our form
(`tools_py/readable_names.py`), with his in-label notes in the last column.

### network: transport, RT, Medius (73)

| r0004 | name (our form) | r0001 twin (how) | his note |
|---|---|---|---|
| `0x001ace10` | `TerminateLibrary` | `0x001ace10` seed+delta |  |
| `0x002471a8` | `sceBInetCreate` | `0x002471a8` exact |  |
| `0x00247988` | `sceBInetControl` | `0x00247988` exact |  |
| `0x0030ea90` | `_roomplayer_sort_aux` | `0x002f1950` exact |  |
| `0x00317f00` | `MediusUpdateAccountStats` | none |  |
| `0x0032a3b0` | `CZNetwork_zNetRegisterRemoteObjectCallback` | none |  |
| `0x0032aa40` | `CZNetwork_zNetGetValidGameClientCount` | `0x0030d510` exact |  |
| `0x003f8e50` | `MediusGetNatServiceAddress` | `0x00646a78` exact |  |
| `0x003f97c8` | `NetGetBuildTimeStamp` | `0x006473f0` exact |  |
| `0x003fa688` | `UpdateStatsRequest` | none |  |
| `0x003fa990` | `CreateGameRequest` | `0x006485b8` relinked-body |  |
| `0x003fb9b8` | `MediusGetPolicy` | none |  |
| `0x0040c874` | `var_MyConnectionIndex` | none |  |
| `0x0061a118` | `NetFreeClientPointers` | `0x006129a0` exact |  |
| `0x0061cdd8` | `NetSendMessageByUDP` | `0x00615660` relinked-body |  |
| `0x0061d070` | `NetISendMessage` | `0x006158f8` relinked-body |  |
| `0x0061e6d8` | `MapMsgClientFailureReason` | none |  |
| `0x006206d8` | `ClientLeaves_sender` | `0x00618f70` relinked-body |  |
| `0x00620858` | `FindFreePingSlot` | `0x006190f0` exact |  |
| `0x006208a0` | `TimeBasePingCallback` | `0x00619138` exact |  |
| `0x00620a08` | `PING` | `0x006192a0` relinked-body |  |
| `0x00620b60` | `NetIPing` | `0x006193f8` relinked-body |  |
| `0x00620d48` | `NetPingIP` | `0x006195e0` exact |  |
| `0x00621be8` | `NetTickWaitTime` | none |  |
| `0x00621e88` | `UpdateField_Sender` | none |  |
| `0x00623470` | `NetUpdateAll` | `0x0061bc68` relinked-body |  |
| `0x00623cd0` | `NetGetClientPingInfo` | `0x0061c510` exact |  |
| `0x006256f0` | `ClientLeaves_Receiver` | `0x0061de18` exact |  |
| `0x00625a00` | `AcceptClientPacketHandler` | `0x0061e128` relinked-body |  |
| `0x00626ce8` | `ArbitrateJoinPacketHandler` | none |  |
| `0x00627168` | `StreamMediaClientInfoPacketHandler` | none |  |
| `0x00628618` | `NetRegisterObjectStart` | `0x00620d28` relinked-body |  |
| `0x0062bef8` | `NetRegisterMessage` | `0x00624418` relinked-body |  |
| `0x00631508` | `KM_MD5` | `0x00629a48` exact |  |
| `0x00631568` | `KM_Salted_MD5` | `0x00629aa8` exact |  |
| `0x00631638` | `md5Update` | `0x00629b78` exact |  |
| `0x00631798` | `md5Final` | `0x00629cd8` exact |  |
| `0x00634dd0` | `rt_comm_get_local_ip` | `0x0062d310` seed+delta |  |
| `0x00634de8` | `rt_comm_get_host_by_name` | `0x0062d328` seed+delta |  |
| `0x006350f8` | `rt_comm_release_channel_id` | `0x0062d638` exact |  |
| `0x00635190` | `rt_comm_is_valid_channel` | `0x0062d6d0` exact |  |
| `0x00635550` | `rt_comm_platform2_startup` | `0x0062da90` relinked-body |  |
| `0x006361e8` | `rt_comm_platform2_display_state` | `0x0062e728` relinked-body |  |
| `0x00636560` | `rt_comm_platform_destroy` | `0x0062eaa0` seed+delta |  |
| `0x006365c8` | `rt_comm_platform_receive` | `0x0062eb08` seed+delta |  |
| `0x006365e0` | `rt_comm_platform_receive_from` | `0x0062eb20` exact |  |
| `0x00636870` | `AppendKeyChain` | `0x0062edb0` exact |  |
| `0x006369f0` | `KM_Salted_SHA1` | `0x0062ef30` exact |  |
| `0x00636e70` | `KM_SetLocalClientID` | `0x0062f3b0` exact |  |
| `0x0063a158` | `rt_msg_client_startup` | none |  |
| `0x0063c350` | `rt_msg_client_idisplay_comm_state` | `0x00634970` relinked-body |  |
| `0x0063da20` | `rt_msg_client_store_msg_in_send_buf` | `0x00636040` relinked-body |  |
| `0x0063dcd0` | `FNC_SortPacketData` | `0x006362f0` relinked-body |  |
| `0x0063dea0` | `rt_msg_client_process_message` | none |  |
| `0x0063ea48` | `rt_msg_client_process_app_message` | `0x00637050` exact |  |
| `0x0063eb48` | `rt_msg_client_process_disconnect_message` | `0x00637150` relinked-body |  |
| `0x0063fdf0` | `rt_msg_client_is_valid_outgoing_msg` | `0x00637f68` exact |  |
| `0x0063fea8` | `rt_msg_client_change_state` | `0x00638020` exact |  |
| `0x00640a68` | `rt_msg_set_2byte` | none |  |
| `0x00640ac8` | `rt_msg_set_u2byte` | `0x00638970` exact |  |
| `0x00640b28` | `rt_msg_set_4byte` | none |  |
| `0x00640bc0` | `rt_msg_set_u4byte` | `0x00638a68` exact |  |
| `0x00641750` | `rt_mutex_platform_get_thread_id` | `0x00639530` exact |  |
| `0x00644fe0` | `rt_thread_count_get_remenant_count` | `0x0063cda0` exact |  |
| `0x00645160` | `rt_time_get_local` | `0x0063cf20` seed+delta |  |
| `0x00645220` | `rt_time_get_time_stamp` | `0x0063cfe0` seed+delta |  |
| `0x00645238` | `rt_time_elapsed` | `0x0063cff8` seed+delta |  |
| `0x00645688` | `rt_time_sleep_usecs` | `0x0063d448` seed+delta |  |
| `0x00645d48` | `rt_time_platform_init` | `0x0063db08` exact (ours there: `rt_time_platform_init`) |  |
| `0x00645da8` | `rt_time_platform_update_base` | `0x0063db68` relinked-body |  |
| `0x00654e80` | `pubKey_0` | none |  |
| `0x00654ec0` | `priKey_1` | none |  |
| `0x00664fd0` | `md5Encode` | `0x00664f80` exact |  |

### game online and session (59)

| r0004 | name (our form) | r0001 twin (how) | his note |
|---|---|---|---|
| `0x001e73e0` | `COurGame_StopPlay` | `0x001e73e0` seed+delta |  |
| `0x001e73f0` | `COurGame_StartPlay` | `0x001e73f0` seed+delta |  |
| `0x001fcd70` | `LoadCameraObjectPlayerObject` | `0x001fcd70` relinked-body |  |
| `0x00224670` | `SetMpStatUiVars` | none |  |
| `0x00226010` | `LoadPlayerScore` | `0x00226010` exact |  |
| `0x0022a290` | `PrintPlayer` | `0x0022a290` relinked-body |  |
| `0x0022de60` | `SortPlayersByScore` | `0x0022de60` relinked-body |  |
| `0x0022f6e0` | `AM_TeamMateCheck` | `0x0022f6e0` relinked-body |  |
| `0x002425e0` | `SpectatorMiniStatsBoard` | `0x002425e0` relinked-body |  |
| `0x0027a480` | `AdvanceMPMission` | none |  |
| `0x0027b8f0` | `CanInvitePlayerToClan` | none |  |
| `0x0027dd00` | `Readytoload` | none |  |
| `0x00280b10` | `GetMapName` | `0x0027f800` seed+delta |  |
| `0x002a0ef0` | `addPlayer_2` | `0x0029f9f0` relinked-body |  |
| `0x002a1110` | `addPlayer` | `0x0029fc10` relinked-body |  |
| `0x002a11b0` | `removePlayer` | `0x0029fcb0` exact |  |
| `0x002a5af0` | `CMission_RecoverFromNetError` | none |  |
| `0x002ab990` | `CMission_DestroyNetGame` | `0x002a9ef0` exact |  |
| `0x002ab9a0` | `CMission_CreateNetGame` | `0x002a9f00` exact |  |
| `0x002ae5e0` | `SendMapNameToServer` | none |  |
| `0x002b4f90` | `LoadPlayerObject` | none |  |
| `0x002b9f70` | `FindHost` | none |  |
| `0x002bb7a0` | `SpectatorVoteReceiver` | `0x002b9bd0` relinked-body |  |
| `0x002bbc10` | `ClientReceiveVote` | none |  |
| `0x002bbd50` | `SendVotePacket` | `0x002ba140` relinked-body |  |
| `0x002befc0` | `CZSealState_RemoveNetSeal` | none |  |
| `0x002bf060` | `GetSealObjFromID` | `0x002bd420` exact |  |
| `0x002c0bc0` | `CZSealState_RemoteSealDeletionCallback` | none |  |
| `0x002c4200` | `CZBombState_RemoteBombCreationCallback` | none |  |
| `0x002c4b60` | `ResolveSpectatorVotes` | `0x002c2ef0` exact |  |
| `0x002c4bc0` | `StoreIncomingVote` | `0x002c2f50` exact |  |
| `0x002c4c10` | `CheckIfSpectator` | `0x002c2fa0` exact |  |
| `0x002c4c40` | `LoadTeamID` | `0x002c2fd0` exact |  |
| `0x002c4d20` | `GetTeamID` | none |  |
| `0x002c51c0` | `CheckBanPlayer` | `0x002c3550` relinked-body |  |
| `0x002c5590` | `ResolveVoteCount` | `0x002c3920` exact |  |
| `0x002c5690` | `LoadPersonaFromIDUpdateVoteCount` | `0x002c3a20` exact |  |
| `0x002c56e0` | `LoadPlayerNameFromID` | `0x002c3a70` exact |  |
| `0x002c5c90` | `CZPersonaState_ToggleReady` | none |  |
| `0x002caff0` | `RoomInfoListMaps` | none |  |
| `0x002ccea0` | `WhisperCheck` | `0x002c9b80` exact |  |
| `0x002d19c0` | `UpdateStatsFields` | none |  |
| `0x002fec70` | `UpdateGameDetails` | `0x002e1e10` exact |  |
| `0x002fed80` | `GetGameDetails` | none |  |
| `0x00302e90` | `CheckclanMemberCount` | `0x002e5fe0` relinked-body |  |
| `0x00307e80` | `SetPlayerRank` | none |  |
| `0x00308300` | `LeaderBoardPlayerRank` | `0x002eb340` relinked-body |  |
| `0x0030d160` | `ViewPlayerProfile` | `0x002f0020` relinked-body |  |
| `0x00315230` | `CZOnlineLobby_SetConnectionInfo` | `0x002f7fa0` relinked-body |  |
| `0x00316a00` | `UpdatePlayerStats` | `0x002f96f0` relinked-body |  |
| `0x003182f0` | `CZOnlineLobby_PasswordSecurity` | `0x002faf50` exact |  |
| `0x0031dc70` | `IsPlayerIgnored` | `0x003008e0` relinked-body |  |
| `0x0032ad00` | `DetermineHost` | none |  |
| `0x003f4060` | `LoadWeaponID_parent` | `0x003d1e10` exact |  |
| `0x0050d460` | `_tosGetProfileFloat` | `0x0050d460` exact |  |
| `0x005496c0` | `UpdateActionCount` | none |  |
| `0x005ee150` | `CRemoteCtrl_Tick` | `0x005e6980` exact |  |
| `0x00604710` | `ControlHostageAction` | `0x005fcf10` exact |  |
| `0x00604810` | `TryControlHostage` | `0x005fd010` exact |  |

### voice and speech (56)

| r0004 | name (our form) | r0001 twin (how) | his note |
|---|---|---|---|
| `0x001e7f58` | `flushbuffer` | `0x001e7f58` exact |  |
| `0x001e8098` | `closemic` | `0x001e8098` exact (ours there: `closemic`) |  |
| `0x001e80f0` | `getsamples` | `0x001e80f0` exact (ours there: `getsamples`) |  |
| `0x002455c0` | `lgAudAEnumHint` | `0x002455c0` relinked-body |  |
| `0x00368b70` | `Headset_StaticSoundSysCallback` | none |  |
| `0x003f71d0` | `mics1` | `0x003d5030` relinked-body |  |
| `0x004d52f8` | `CreateCtxTree` | `0x004d52f8` exact |  |
| `0x004decc0` | `CalcSentenceConfidence` | `0x004decc0` exact |  |
| `0x004df258` | `cbNewUserWordUtterance` | `0x004df258` exact |  |
| `0x00509618` | `_cdbGetProbMatrixSize` | `0x00509618` exact |  |
| `0x005096e0` | `GetNbrStatesInPhenome` | `0x005096e0` exact |  |
| `0x00509700` | `ConvPhenomesToStates` | `0x00509700` exact |  |
| `0x00655658` | `iARCHI_FLUSH_IGNORE` | `0x00656b58` exact |  |
| `0x006556c8` | `iARCHI_FLUSH` | `0x00656bc8` exact |  |
| `0x00655700` | `iARCHI_GOTOSLEEP` | `0x00656c00` exact |  |
| `0x00655760` | `iARCHI_START_DETECTION` | `0x00656c60` exact |  |
| `0x00655780` | `iARCHI_NO_START_DETECTION` | `0x00656c80` exact |  |
| `0x00655810` | `iARCHI_DISABLE_INPUT_F` | `0x00656d10` exact |  |
| `0x00655820` | `iARCHI_DISABLE_INPUT_C` | `0x00656d20` exact |  |
| `0x00655830` | `iARCHI_ENABLE_M` | `0x00656d30` exact |  |
| `0x00655858` | `iARCHI_ENABLE_F` | `0x00656d58` exact |  |
| `0x00655880` | `iARCHI_ENABLE_C` | `0x00656d80` exact |  |
| `0x006558a8` | `iARCHI_STOP_SPECSUB_SIL_QUEUE` | `0x00656da8` exact |  |
| `0x006558b8` | `iARCHI_START_SPECSUB_SIL_QUEUE` | `0x00656db8` exact |  |
| `0x00655948` | `iARCHI_GET_DISTCALCFUNCS` | `0x00656e48` exact |  |
| `0x00655958` | `iARCHI_SET_DISTCALCFUNCS` | `0x00656e58` exact |  |
| `0x00655968` | `iARCHI_GET_PSIGNALLEVEL` | `0x00656e68` exact |  |
| `0x00655988` | `iARCHI_GET_PBEGINTIME` | `0x00656e88` exact |  |
| `0x00655998` | `iARCHI_GET_EXTRAEVENTSCORE` | `0x00656e98` exact |  |
| `0x006559a8` | `iARCHI_VAD_SET_PARAM` | `0x00656ea8` exact |  |
| `0x006559b8` | `iARCHI_TRIGGER_SET_PARAM` | `0x00656eb8` exact |  |
| `0x006559c8` | `iARCHI_GENDER_SET_PARAM` | `0x00656ec8` exact |  |
| `0x006559d8` | `iARCHI_DISTCALC_SET_PARAM` | `0x00656ed8` exact |  |
| `0x006559f8` | `iARCHI_VAD_GET_PARAM` | `0x00656ef8` exact |  |
| `0x00655a08` | `iARCHI_TRIGGER_GET_PARAM` | `0x00656f08` exact |  |
| `0x00655a18` | `iARCHI_GENDER_GET_PARAM` | `0x00656f18` exact |  |
| `0x00655a28` | `iARCHI_DISTCALC_GET_PARAM` | `0x00656f28` exact |  |
| `0x00655a48` | `iARCHI_LOADLANGUAGE` | `0x00656f48` exact |  |
| `0x00655a90` | `iARCHI_CHANGECTX` | `0x00656f90` exact |  |
| `0x00655ad0` | `iARCHI_GETACTWORDS` | `0x00656fd0` exact |  |
| `0x00655ae0` | `iARCHI_MIKE_SET` | `0x00656fe0` exact |  |
| `0x00655bd0` | `iARCHI_INITSCOREUSERWORD` | `0x006570d0` exact |  |
| `0x00655be0` | `iARCHI_SETPARAMSCOREUSERWORD` | `0x006570e0` seed+delta |  |
| `0x00655bf0` | `iARCHI_GETPARAMSCOREUSERWORD` | `0x006570f0` seed+delta |  |
| `0x00655c00` | `iARCHI_SILADATPTON` | `0x00657100` exact |  |
| `0x00655c30` | `iARCHI_LABEL_START_SECONDPASS` | `0x00657130` exact |  |
| `0x00655c60` | `iARCHI_LABEL_STOP_UW` | `0x00657160` exact |  |
| `0x00655c70` | `iARCHI_NO_PITCH_PROCESSING` | `0x00657170` exact |  |
| `0x00655c90` | `iARCHI_DO_PITCH_PROCESSING` | `0x00657190` exact |  |
| `0x00655ee8` | `iARCHI_SWITCH_TO_NBESTDP` | `0x006573e8` exact |  |
| `0x00655f00` | `iARCHI_SWITCH_TO_TREEDP` | `0x00657400` exact |  |
| `0x00655f18` | `iARCHI_SWITCH_TO_GENUWDP` | `0x00657418` exact |  |
| `0x00656ad0` | `Vq_KeyVals_U32` | none |  |
| `0x006585d0` | `aQRSpline8kHz` | `0x00659ad0` seed+delta |  |
| `0x006587f0` | `aBeta8kHz` | `0x00659cf0` seed+delta |  |
| `0x00658b58` | `aQRSpline11kHz` | `0x00657948` seed+delta |  |

### game classes (mangled) (130)

| r0004 | name (our form) | r0001 twin (how) | his note |
|---|---|---|---|
| `0x001f1e10` | `CEntityItr_GetSeal` | `0x001f1e10` exact |  |
| `0x001f5d00` | `CLoadState_AbortLoad` | `0x001f5d00` exact |  |
| `0x001fd8c0` | `CInGameWeaponSel_dtor` | `0x001fd8c0` exact |  |
| `0x00208ab0` | `arraydtor_2524` | `0x00208ab0` seed+delta |  |
| `0x0020f100` | `anon_hud_linemap_CAiLinePtItr_FindValid` | `0x0020f100` exact |  |
| `0x00210a70` | `anon_hud_linemap_CAiLinePtItr_GetPt` | `0x00210a70` exact |  |
| `0x0021b3e0` | `CWeaponSel_OnClose` | `0x0021b3e0` exact |  |
| `0x0021f850` | `CActionTxtrMachine_Open` | `0x0021f850` relinked-body | zero object interaction? |
| `0x00226020` | `CMPExitMenu_GetReaderFileName` | `0x00226020` exact |  |
| `0x00227650` | `zdb_CNode_DeleteChild` | `0x00227650` exact |  |
| `0x00227960` | `CInGameWeaponSel_UnInit` | `0x00227960` exact |  |
| `0x00258c90` | `zar_CZAR_OpenKey` | `0x00258650` exact |  |
| `0x00259280` | `zar_CZAR_Close` | `0x00258c40` exact |  |
| `0x0025e0c0` | `zAnimCallAnimTick` | `0x0025d550` relinked-body |  |
| `0x00264610` | `zAnimObjectActiveStateTick` | `0x00263aa0` relinked-body |  |
| `0x002694a0` | `CZAnimMain_AnimSchedulerTick` | `0x00268930` exact |  |
| `0x0026a970` | `CZAnimMain_SeqTick` | `0x00269da0` exact |  |
| `0x0026b460` | `CZAnimMain_CmdNext` | `0x0026a890` exact |  |
| `0x0026b480` | `CZAnimMain_CmdNext` | `0x0026a8b0` exact |  |
| `0x0026c1b0` | `CZAnimSetSave_Add` | `0x0026b5e0` exact |  |
| `0x0026d990` | `CZAnimSet_TaskScheduler` | `0x0026cbe0` exact |  |
| `0x00273970` | `CZAnimArgNode_ctor` | `0x00272a60` exact |  |
| `0x0027dae0` | `CPopUpDialog_dtor` | `0x0027c8d0` exact |  |
| `0x002881b0` | `PlaneConstant` | `0x00286e20` exact |  |
| `0x0029f5d0` | `CConsole_Initalize` | none |  |
| `0x0029fe70` | `CEntity_TeleportTo` | `0x0029e970` relinked-body |  |
| `0x0029fff0` | `CEntity_TeleportTo` | `0x0029eaf0` relinked-body |  |
| `0x002a01e0` | `CEntity_JoinTeam` | `0x0029ece0` exact |  |
| `0x002a7840` | `CMission_Serialize` | `0x002a6340` exact |  |
| `0x002aec90` | `CMission_Init` | none |  |
| `0x002b4e40` | `CMission_CloseVehicleRdr` | `0x002b3430` exact |  |
| `0x002b69b0` | `CNodeAction_Open` | none |  |
| `0x002b71d0` | `_actionValveCallback` | none |  |
| `0x002c0b80` | `CZSealState_SealDeleteCleanup` | `0x002bef20` exact |  |
| `0x002c36f0` | `CZBombState_InitBomb` | none |  |
| `0x002c3860` | `CZBombState_CreateBombHere` | `0x002c1bf0` relinked-body |  |
| `0x002c3d00` | `CZBombState_RequestNewBombState` | `0x002c2090` relinked-body |  |
| `0x002f33b0` | `zdb_CGrid_gridGetGridXandZIndex` | `0x002d6380` exact |  |
| `0x002f3640` | `zdb_CGrid_StartTraversalOrdered` | `0x002d6610` exact |  |
| `0x002f3670` | `zdb_CGrid_StartTraversal` | `0x002d6640` exact |  |
| `0x002f64c0` | `zdb_CClutter_GetTranslate` | `0x002d9490` exact |  |
| `0x002f7190` | `CPad_SetButtonState` | `0x002da160` exact |  |
| `0x002f9170` | `zdb_IntersectStruct_IsInside` | `0x002dc180` exact |  |
| `0x002f92d0` | `zdb_CDI_Create` | `0x002dc2e0` exact |  |
| `0x002fa680` | `intersect2d` | `0x002dd690` exact |  |
| `0x002fab20` | `zdb_CDIBBox_ctor` | `0x002ddb30` exact |  |
| `0x002fbb20` | `CMaterialEx_ctor` | `0x002deb30` exact |  |
| `0x003254f0` | `CMatrix_SetZero` | `0x00308150` exact |  |
| `0x00326520` | `CPnt3D_Scale` | `0x00309180` exact |  |
| `0x00331fb0` | `zdb_CNode_InsertAtom` | `0x00314a10` exact |  |
| `0x003320a0` | `zdb_CNode_AllocateAtomBasePtrs` | `0x00314b00` exact |  |
| `0x00332c40` | `zdb_CNode_GetRotation` | `0x003156a0` exact |  |
| `0x00333650` | `zdb_CNode_ReserveChildren` | `0x003160b0` seed+delta |  |
| `0x00333660` | `zdb_CNode_RemoveFromParent` | `0x003160c0` exact |  |
| `0x00335090` | `zdb_CNode_CreateXRef` | `0x00317af0` exact |  |
| `0x00336fd0` | `zdb_CSaveLoad_LoadAssetLib_PS2` | `0x003199c0` exact |  |
| `0x00337750` | `zdb_CSaveLoad_ctor` | `0x0031a140` exact |  |
| `0x00338740` | `CWind_Reset` | `0x0031b130` exact |  |
| `0x0033ae30` | `zdb_CWorld_Segregate_CCells` | `0x0031d820` exact |  |
| `0x0033b350` | `zdb_CWorld_ClearIgnoreNodeDelayed` | `0x0031dd40` exact |  |
| `0x0033b370` | `zdb_CWorld_RegIgnoreNodeDelayed` | `0x0031dd60` exact |  |
| `0x0033b560` | `zdb_CWorld_GetAltitude` | `0x0031df50` exact |  |
| `0x00346370` | `CPnt4D_ctor` | `0x00328d60` exact |  |
| `0x0035a7e0` | `CSaveModule_SaveRdr` | `0x0033d250` exact |  |
| `0x00367100` | `CSndSequenceEntry_IsFading` | `0x00349a70` exact |  |
| `0x00367380` | `CSndSequence_IsFading` | `0x00349cf0` exact |  |
| `0x0036f5e0` | `CValve_Destroy` | `0x00351fb0` exact |  |
| `0x00388250` | `ActionItem_ctor` | `0x00369f70` exact |  |
| `0x00388ec0` | `zUISchedulerClear` | `0x0036abe0` exact |  |
| `0x0038b810` | `CUIMapCtrl_CalcAlternateZoomedPos` | `0x0036d510` exact |  |
| `0x0039be80` | `CDesignDlgObj_op_assign` | `0x0037d910` exact |  |
| `0x003a5ff0` | `CUIVarManager_dtor` | `0x00385ce0` exact |  |
| `0x003cb0b0` | `CTurret_SampleDynamics` | `0x003a8f50` relinked-body |  |
| `0x003f6800` | `CZAmmo_SetHitAnim` | `0x003d45b0` exact |  |
| `0x003f77a0` | `CFlyoutMachine_Init` | `0x003d5600` exact |  |
| `0x00422030` | `CMatrix_identity` | none |  |
| `0x00422090` | `CPnt3D_unit_x` | none |  |
| `0x0042c7b0` | `sinit_ent_main` | `0x003fffb0` relinked-body (ours there: `sinit_ent_main`) |  |
| `0x0042ee60` | `sinit_zmath_main` | `0x004025e0` relinked-body (ours there: `sinit_zmath_main`) |  |
| `0x0042eeb0` | `sinit_zmath_matrix` | `0x00402630` exact (ours there: `sinit_zmath_matrix`) |  |
| `0x0042ef40` | `sinit_zmath_pnt3d` | `0x004026c0` relinked-body (ours there: `sinit_zmath_pnt3d`) |  |
| `0x0042efd0` | `sinit_zmath_pnt4d` | `0x00402750` relinked-body (ours there: `sinit_zmath_pnt4d`) |  |
| `0x00432a40` | `CPacket_vt` | none |  |
| `0x00433cd0` | `CPopUpDialog_vt` | none | cpopupdialog |
| `0x004348e0` | `CButtonSpec_vt` | none |  |
| `0x0050df10` | `ai_elevate` | `0x0050df10` exact |  |
| `0x0050f510` | `CAiEvent_GetEventRate` | `0x0050f510` exact |  |
| `0x00510970` | `std_find` over a `list<CAiVisMap*>` (template, shortened) | `0x00510970` exact |  |
| `0x00517520` | `CAiMap_GetMapRegion` | `0x005173e0` exact |  |
| `0x00517ff0` | `CAiMap_validate_cell` | `0x00517eb0` exact |  |
| `0x005182c0` | `CAiMap_reset_rtflag` | `0x00518180` exact |  |
| `0x00519160` | `CAiMap_GetDirQuat` | `0x00518ea0` exact |  |
| `0x0051b1b0` | `CAiMap_read_scanlines` | none |  |
| `0x00527c30` | `CAiPath_plan_setup` | `0x00527970` exact |  |
| `0x00531b30` | `CAiMaps_dtor` | `0x0052ea10` exact |  |
| `0x00531fc0` | `CAiMap_DestroyGraph` | `0x0052eea0` exact |  |
| `0x00533af0` | `std_vector_imp<CAiPath::point>_back` (shortened) | `0x00530980` exact |  |
| `0x005460a0` | `CPickup_SetVisibleByUnit` | `0x00541650` exact |  |
| `0x00546130` | `CPickup_GetPosW` | `0x005416e0` exact |  |
| `0x00548640` | `CZSealBody_UnMakeGhost` | `0x00543b90` exact |  |
| `0x00559180` | `CZSealBody_OnMissionStart` | `0x00553e60` exact |  |
| `0x0055a3f0` | `zdb_DiIntersect_defctor` | none |  |
| `0x00567500` | `CSealCtrlAi_GetValve` | `0x005621c0` exact |  |
| `0x00567e10` | `CSealCtrlAi_SetupAnimations` | `0x00562ad0` exact |  |
| `0x00572df0` | `CSealCtrl_SetGoalPos` | `0x0056daa0` exact |  |
| `0x005730a0` | `CSealCtrlAi_ResetToInitialState` | `0x0056dd50` exact |  |
| `0x00573110` | `CSealCtrlAi_OnRecycleEntity` | `0x0056ddc0` exact |  |
| `0x00579bc0` | `CSealCtrlAi_GetAiStateEnum` | `0x00574800` exact |  |
| `0x00597c30` | `CPnt3D_Cross` | `0x00592400` exact |  |
| `0x005984a0` | `CSealCtrl_SetCameraZoom` | `0x00592c70` exact |  |
| `0x005a3a10` | `CZSealBody_EnableMPBombGear` | `0x0059df60` exact |  |
| `0x005af0b0` | `CZSealBody_FindDeathAnimTypeCQB` | `0x005a9420` relinked-body |  |
| `0x005b0100` | `CNodeAction_GetAction` | `0x005aa470` exact |  |
| `0x005b0320` | `CZSealBody_GetReticuleSeal` | `0x005aa690` exact | 1 |
| `0x005b4170` | `CLook_ctor` | `0x005ae450` exact |  |
| `0x005bff10` | `CZKit_IsUsingItem` | `0x005b97a0` exact |  |
| `0x005c3be0` | `CZKit_FireGrenade` | `0x005bd030` exact | to spawnProjectile c4 check |
| `0x005c49d0` | `CZKit_SetItemAmmo` | `0x005bde20` exact |  |
| `0x005c4a70` | `CZKit_SetItemAmmo` | `0x005bdec0` exact | set ammo count fnc |
| `0x005dced0` | `CTPaint_Tick` | `0x005d5700` exact |  |
| `0x005e6660` | `CZSealBody_GetAimDot` | `0x005dee90` exact |  |
| `0x005e6cb0` | `CZSealBody_SetAimPointW` | `0x005df4e0` exact |  |
| `0x005e7080` | `CZSealBody_SetAimDirR` | `0x005df8b0` relinked-body |  |
| `0x005e7790` | `CZSealBody_WepWaitOn` | `0x005dffc0` exact |  |
| `0x005e9170` | `CAnimsByFP_SetPistolAnim` | `0x005e19a0` exact |  |
| `0x005eee30` | `CSealUnit_ResetPointOutState` | `0x005e7660` exact |  |
| `0x005eee60` | `CSealUnit_UpdatePointOutState` | `0x005e7690` exact |  |
| `0x005f6750` | `CSealUnit_GetMyLookDir` | `0x005eef80` exact |  |
| `0x006024e0` | `CSealCtrlAi_SetAiEvent` | `0x005fad00` exact |  |
| `0x0060c030` | `CAiSFlee_OnSignal` | `0x00604830` exact |  |

### render, HUD and menus (mangled) (37)

| r0004 | name (our form) | r0001 twin (how) | his note |
|---|---|---|---|
| `0x001f3bd0` | `CMenuState_RestartBackgroundMPEG` | `0x001f3bd0` seed+delta |  |
| `0x001f3be0` | `CMenuState_HaltBackgroundMPEG` | `0x001f3be0` seed+delta |  |
| `0x001f7e50` | `CHUD_PostDITick` | `0x001f7e50` exact |  |
| `0x001f7e70` | `CHUD_PreDITick` | `0x001f7e70` exact |  |
| `0x001fba30` | `GetTextureByName` | `0x001fba30` exact |  |
| `0x001fdbe0` | `CPlainBmp_dtor` | `0x001fdbe0` exact |  |
| `0x00208ba0` | `CZClaymoreMapItem_Draw` | `0x00208ba0` seed+delta |  |
| `0x0020cff0` | `CZMPBombMapItem_Draw` | `0x0020cff0` seed+delta |  |
| `0x00212140` | `MapCompass_SetTrans` | `0x00212140` seed+delta |  |
| `0x00212160` | `MapCompass_SetTexture` | `0x00212160` exact |  |
| `0x00222f10` | `CZPlayerMapItem_PostDraw` | `0x00222f10` exact |  |
| `0x00228b50` | `CFader_SetMinBrightness` | `0x00228b50` exact |  |
| `0x002388a0` | `std_vector_deleter<Particle::SequenceEntry<CPnt4D>>_dtor` (shortened) | `0x002388a0` relinked-body |  |
| `0x0023d810` | `C2DBitmapPoly_SetTrans` | `0x0023d810` exact |  |
| `0x002fabe0` | `zdb_CDIPoly_ctor` | `0x002ddbf0` exact |  |
| `0x00344bc0` | `Particle_Instance_Expire` | `0x003275b0` exact |  |
| `0x00347e70` | `Particle_Source_ScaleSequenceAdd` | `0x0032a860` exact |  |
| `0x00348030` | `Particle_Source_TextureSequenceAdd` | `0x0032aa20` exact |  |
| `0x003766d0` | `C2DBitmapPoly_SetUseFrameBufferAlpha` | `0x00358ef0` exact |  |
| `0x0037d350` | `clip3DLine` | `0x0035fac0` exact |  |
| `0x0037e890` | `C2DPoly_SetUV` | `0x00361000` exact |  |
| `0x0037ea90` | `C2DPoly_SetColor` | `0x00361200` exact |  |
| `0x00380450` | `C2DString_UpdatePos` | `0x00362b70` exact |  |
| `0x00381f60` | `C2DBitmap_SetPos` | `0x00364680` exact |  |
| `0x00382310` | `C2DBitmap_Draw` | `0x00364a30` exact |  |
| `0x00384d40` | `CGameMenu_HaltBackgroundMPEG` | `0x00366b10` exact |  |
| `0x003892c0` | `CRenderableString_SetMapOffset` | `0x0036afc0` exact |  |
| `0x0038e730` | `CScrollText_Tick` | `0x00370280` exact |  |
| `0x0038fa40` | `CCheckBox_LoadSpec` | `0x00371550` exact |  |
| `0x0039a8d0` | `C2DButton_SetBackgroundAlpha` | `0x0037c2c0` exact |  |
| `0x003a7d50` | `CCheckBoxSpec_Copy` | `0x003879e0` exact |  |
| `0x003ae8e0` | `anon_zui_skb_CBmpWithNode_dtor` | `0x0038e490` exact |  |
| `0x003d5aa0` | `zdb_CVisual_ApplyDecalRender` | `0x003b3950` exact |  |
| `0x003dfaf0` | `zdb_CMeshDecalPool_Uninit` | `0x003bd9a0` exact |  |
| `0x00433720` | `C2DBitmap_vt` | none |  |
| `0x00434280` | `CPlainBmp_vt` | none |  |
| `0x0059e360` | `ai_DrawFunc11CDynGrenade` | `0x00598860` exact |  |

### game logic, his readings (34)

| r0004 | name (our form) | r0001 twin (how) | his note |
|---|---|---|---|
| `0x001ff060` | `LensFlareManager` | `0x001ff060` exact |  |
| `0x002b72a0` | `PrintMessageFeed` | `0x002b5830` relinked-body |  |
| `0x002b7fa0` | `PrintMessageToMessageFeed` | `0x002b6530` relinked-body |  |
| `0x002bc9d0` | `AllSwitch` | `0x002badc0` seed+delta |  |
| `0x00308660` | `GetLadderInfo` | `0x002eb6a0` relinked-body |  |
| `0x00315500` | `HOST_UpdateMapInfo` | `0x002f8270` exact |  |
| `0x0033d320` | `SpawnWeaponObject` | `0x0031fd10` seed+delta |  |
| `0x00350990` | `SetValue` | `0x003333f0` hash+callees |  |
| `0x00350a30` | `SetColor` | `0x00333490` exact |  |
| `0x0035da70` | `SetSoundLevel` | `0x003404e0` seed+delta |  |
| `0x00380ea0` | `PrintString` | `0x003635c0` relinked-body |  |
| `0x003e6d90` | `GetWeaponID` | `0x003c4c10` exact |  |
| `0x003ed340` | `SpawnProjectileObject` | `0x003cb1a0` relinked-body |  |
| `0x003f13f0` | `GetWeaponInfo` | `0x003cf1f0` exact |  |
| `0x003f3bf0` | `GetWeaponName` | `0x003d19a0` exact |  |
| `0x003f3cb0` | `GetCrosshairType` | `0x003d1a60` exact |  |
| `0x003f4a90` | `GetWeaponName` | `0x003d2840` seed+delta |  |
| `0x0040cb38` | `CZFTSWeapon` | none |  |
| `0x00424c50` | `c_arrAnchors_3348` | none |  |
| `0x00434d20` | `PlacedExplosive` | none |  |
| `0x00434d30` | `CZFTSWeapon` | none |  |
| `0x00547f60` | `CreateRadarObject` | `0x005434b0` relinked-body |  |
| `0x0058e160` | `PlayAnimation` | none |  |
| `0x00592550` | `GetPlayerDynamicsStart` | `0x0058ce60` seed+delta |  |
| `0x005a50c0` | `PickupWeapon` | `0x0059f610` relinked-body |  |
| `0x005c0100` | `DisplayBullets` | `0x005b9990` exact |  |
| `0x005c4b60` | `weaponSetUpWeaponsForSpawn` | `0x005bdfb0` exact |  |
| `0x005c77f0` | `HandleWeaponStuff` | `0x005c0ae0` relinked-body |  |
| `0x005cdee0` | `GetWeaponObject` | `0x005c67d0` exact |  |
| `0x005cfd50` | `DoesPlayerHaveItem` | `0x005c8640` exact |  |
| `0x0064ee50` | `qlookspinelo` | none |  |
| `0x0064ef30` | `qlookneck` | none |  |
| `0x0064efa0` | `qlookhead` | none |  |
| `0x00668e30` | `CSealCtrl` | none |  |

### SDK, libc and middleware (72)

| r0004 | name (our form) | r0001 twin (how) | his note |
|---|---|---|---|
| `0x00180e10` | `malloc` | `0x00180e10` exact |  |
| `0x00181c50` | `_dpfgt` | `0x00181c50` exact |  |
| `0x00181c70` | `_dpflt` | `0x00181c70` exact |  |
| `0x00182810` | `register_global_object` | `0x00182810` exact |  |
| `0x00184260` | `FindExceptionTable` | `0x00184260` exact |  |
| `0x0018aa30` | `replace_Q23std15_locale_imp1FRCQ33std6locale5facetUi` | `0x0018aa30` exact |  |
| `0x0018d250` | `do_widen_Q23std8ctypecCFPCcPCcPc` | `0x0018d250` exact |  |
| `0x0018d290` | `do_narrow_Q23std8ctypecCFPCcPCccPc` | `0x0018d290` exact |  |
| `0x0018ed78` | `sceCdDiskReady` | `0x0018ed78` exact |  |
| `0x001980a0` | `isnan` | `0x001980a0` exact |  |
| `0x001983e0` | `copysignf` | `0x001983e0` exact |  |
| `0x001a3910` | `iReleaseAlarm` | none |  |
| `0x001a3950` | `ExitThread` | none |  |
| `0x001a3980` | `iTerminateThread` | none |  |
| `0x001a3ac0` | `iResumeThread` | none |  |
| `0x001a45d8` | `iInvalidDCache` | none |  |
| `0x001a7918` | `_sceFsSigSema` | `0x001a7918` exact |  |
| `0x001ab9c8` | `sceSifLoadModule` | `0x001ab9c8` exact |  |
| `0x001ac2f0` | `InitTLB32MB` | `0x001ac2f0` exact (ours there: `InitTLB32MB`) |  |
| `0x001ac600` | `_xlaunch` | none |  |
| `0x001aced0` | `Exit` | `0x001aced0` exact |  |
| `0x001b28f8` | `kernel_sinf` | `0x001b28f8` exact (ours there: `kernel_sinf`) |  |
| `0x001b82d8` | `_ipuVdec` | `0x001b82d8` exact (ours there: `_ipuVdec`) |  |
| `0x001b9280` | `_updateRefImage` | `0x001b9280` exact (ours there: `_updateRefImage`) |  |
| `0x001bae00` | `sceMpegDemuxPss` | `0x001bae00` exact |  |
| `0x001bbd90` | `_getpic` | `0x001bbd90` exact (ours there: `_getpic`) |  |
| `0x001bc228` | `_initSeqAgain` | `0x001bc228` exact |  |
| `0x001bc3e0` | `ErrMessage` | `0x001bc3e0` relinked-body |  |
| `0x001bd320` | `sceSifMCallRpc` | `0x001bd320` exact (ours there: `sceSifMCallRpc`) |  |
| `0x001bfc00` | `sceVu0ApplyMatrix` | `0x001bfc00` exact |  |
| `0x001bfc98` | `sceVu0InnerProduct` | `0x001bfc98` exact |  |
| `0x001bfcc0` | `sceVu0Normalize` | `0x001bfcc0` exact |  |
| `0x001bfdb8` | `sceVu0DivVector` | none |  |
| `0x001bfdd8` | `sceVu0DivVectorXYZ` | none |  |
| `0x001bfdf8` | `sceVu0InterVector` | `0x001bfdf8` exact |  |
| `0x001bfe80` | `sceVu0TransMatrix` | `0x001bfe80` exact |  |
| `0x001bfec0` | `sceVu0CopyMatrix` | `0x001bfec0` exact |  |
| `0x001bff28` | `sceVu0UnitMatrix` | `0x001bff28` exact |  |
| `0x001c0210` | `sceVu0ClampVector` | none |  |
| `0x001c0410` | `sceVu0ViewScreenMatrix` | `0x001c0410` exact (ours there: `sceVu0ViewScreenMatrix`) |  |
| `0x001c0768` | `sceVu0InterVectorXYZ` | `0x001c0768` exact |  |
| `0x001c2160` | `thread_enter0` | `0x001c2160` exact |  |
| `0x001ca610` | `std_lower_map` | none |  |
| `0x001ca710` | `std_upper_map` | none |  |
| `0x001cc428` | `dch` | none |  |
| `0x001ccc00` | `init_mp3_3` | none |  |
| `0x001ccc20` | `iqval` | none |  |
| `0x001ccc70` | `vqval` | none |  |
| `0x001ccd78` | `tlbsrc` | `0x001ccd78` exact |  |
| `0x001cd3e8` | `osdsrc` | `0x001cd3e8` exact |  |
| `0x001cdbb0` | `srcfile` | `0x001cdbb0` exact |  |
| `0x001ce2f0` | `eenull` | none |  |
| `0x001ce4c0` | `_defIQM` | none |  |
| `0x001ce500` | `_defNIQM` | none |  |
| `0x001d0880` | `stdtbl` | none |  |
| `0x001d0890` | `mfdtbl` | none |  |
| `0x001d0cd0` | `zeroes_13` | none |  |
| `0x001d2bd8` | `PIo2` | none |  |
| `0x001d2d48` | `PIo2` | none |  |
| `0x001d2d98` | `T` | none |  |
| `0x001d2e00` | `aT` | none |  |
| `0x00328af0` | `loadImage` | `0x0030b740` exact |  |
| `0x0035c7b0` | `snd_AutoReverb` | `0x0033f220` exact |  |
| `0x0035d660` | `snd_AutoVol` | `0x003400d0` exact |  |
| `0x0035da00` | `snd_GetPlaybackMode` | `0x00340470` seed+delta |  |
| `0x003bc660` | `CBufferIO_ftell` | `0x0039c000` exact |  |
| `0x003bc940` | `CBufferIO_ctor` | `0x0039c2e0` exact |  |
| `0x004017b0` | `iTerminateString` | `0x0064f3d8` exact |  |
| `0x004068f8` | `ZMINUSLINE` | none |  |
| `0x0040813c` | `threadsafeSema` | none |  |
| `0x0040c440` | `zon_writeon_dmaref` | none |  |
| `0x004f70a0` | `mtxAssociateAll` | `0x004f70a0` exact |  |

### his purpose notes and scratch labels (51)

| r0004 | name (our form) | r0001 twin (how) | his note |
|---|---|---|---|
| `0x001efa50` | `lightfnc1` | `0x001efa50` relinked-body |  |
| `0x001efc30` | `lightfnc2` | `0x001efc30` relinked-body |  |
| `0x001f0030` | `scopes1` | `0x001f0030` relinked-body |  |
| `0x00211510` | `gamedynamics1` | `0x00211510` relinked-body |  |
| `0x0021a700` | `shootrockets` | `0x0021a700` exact |  |
| `0x002269a0` | `etags` | `0x002269a0` relinked-body |  |
| `0x0022ac30` | `save` | `0x0022ac30` relinked-body |  |
| `0x0022f0f0` | `votespec` | `0x0022f0f0` relinked-body |  |
| `0x0022f3c0` | `voteflood` | `0x0022f3c0` relinked-body |  |
| `0x00249958` | `clipping2` | `0x00249b58` exact |  |
| `0x00264e60` | `lightfnc3` | `0x002642f0` relinked-body |  |
| `0x00275400` | `armory3` | `0x002744f0` exact |  |
| `0x0027e280` | `force` | none |  |
| `0x00282c60` | `weapons2` | `0x00281950` exact |  |
| `0x0028af40` | `sysfnc1` | `0x00289bb0` relinked-body |  |
| `0x0028bd80` | `gamespeed1` | `0x0028a9f0` relinked-body |  |
| `0x002a8af0` | `sysfnc2` | none |  |
| `0x002bb730` | `specboot` | `0x002b9b60` relinked-body |  |
| `0x002c0a70` | `parent` | `0x002bee10` relinked-body |  |
| `0x002c5960` | `startgame1` | none |  |
| `0x002f6ad0` | `foilage` | `0x002d9aa0` exact |  |
| `0x00311020` | `nothing` | `0x002f3ee0` seed+delta |  |
| `0x00336340` | `camera` | `0x00318da0` relinked-body |  |
| `0x0034c970` | `color1` | `0x0032f350` exact |  |
| `0x00350ca0` | `unk1` | `0x00333700` exact |  |
| `0x00350d10` | `unk3` | `0x00333770` exact |  |
| `0x00350f50` | `unk2` | `0x003339b0` exact |  |
| `0x00375000` | `foilage2` | `0x00357820` exact |  |
| `0x00382730` | `disable` | `0x00364e50` exact |  |
| `0x003ba540` | `textlimit` | `0x00399ec0` relinked-body |  |
| `0x003e7b00` | `scopes2` | `0x003c5980` exact |  |
| `0x003ebb30` | `weapon1` | `0x003c99a0` relinked-body |  |
| `0x003efbd0` | `dynamics2` | none |  |
| `0x003f0fb0` | `dynamics1` | `0x003cedb0` relinked-body |  |
| `0x003f3c30` | `armory2` | `0x003d19e0` exact |  |
| `0x003fe6e8` | `whisper` | none |  |
| `0x0040b830` | `ascii_special` | none |  |
| `0x0041fa58` | `d` | none |  |
| `0x00432a00` | `cappcamera` | none |  |
| `0x00433890` | `texturerelocator` | none |  |
| `0x0054cf30` | `clipping` | none |  |
| `0x005a61b0` | `health3` | `0x005a0700` exact |  |
| `0x005a7650` | `health2` | `0x005a1b70` exact |  |
| `0x005a8200` | `death1` | `0x005a2690` relinked-body |  |
| `0x005ab040` | `health4` | `0x005a54d0` exact |  |
| `0x005b5660` | `jump1` | none |  |
| `0x005c4620` | `infammo` | `0x005bda70` exact |  |
| `0x005c8860` | `infrockets` | none |  |
| `0x005ca830` | `recoil1` | `0x005c3360` relinked-body |  |
| `0x005d0cf0` | `fnc` | `0x005c9520` relinked-body |  |
| `0x005e95c0` | `anims` | `0x005e1df0` relinked-body |  |
