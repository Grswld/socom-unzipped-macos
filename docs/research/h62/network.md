# h62 -- the network layer

*2026-10-11 22:39Z. Part of the h62 set ([README](README.md)): Harry62's findings, credited to him and restated in
our words. Every h62 claim is believed until a read or probe of ours stands behind it; KNOWN wins.*

This is where his work is strongest. He has spent years inside SOCOM II's online code (the r0005 patch, his chat and
anti-cheat work), and his labels name the online stack layer by layer. Below, the stack from the wire up, each layer
with what he named, what we already had, and where the two meet.

## The stack in one picture

| layer | r0004 range (approx.) | what it is | his names | ours before h62 |
|---|---|---|---|---|
| libnetb (EE side of the network IOP module) | `0x245600`-`0x248000` | UDP/TCP send and receive through the IOP RPC (research/10) | 7, all agreeing (2 as short aliases) | named by hand: the runtime hooks these (`socom2_addresses.h`) |
| RT message client (SCE-RT, the DME transport) | `0x633c00`-`0x646000` | connections, message framing, dispatch, timing, mutexes | 29 new + 55 agreeing | 78 `rt_*` rows, mostly `exact` from the demo |
| keys and hashes | `0x631500`-`0x637600`, `0x654e80`, `0x664f80` | `KM_*` key management, MD5 / SHA1 for session keys | 10 new | `KM_GetGameSessionKey`, `KM_GetSoftwareKeyPair` |
| the game's net layer (`NetI*`) | `0x61a000`-`0x62c000` | send by UDP/TCP, ping, client bookkeeping, object and message registration, packet handlers | 20 new + 7 agreeing | 7 (`SendPacket`, `NetSendMessageByTCP`, `NetInitialize` ...) |
| Medius client wrappers | `0x3f8000`-`0x3fc000`, `0x317f00` | lobby, stats, policy, NAT service | 6 new + 2 agreeing | the Medius request names from the demo |
| the game's own packet code (Zipper's C++) | `0x2a5000`-`0x32b000` | `CZNetwork`, `CMission` net game, `CZSealState`, `CZBombState`, `CZOnlineLobby` | 10 new (mangled) + 2 agreeing | a handful |
| session logic (his readings) | `0x22f000`-`0x32ad00` | votes, bans, host selection, whisper, stats upload | ~40 new | almost none |

Counts are function starts in `recomp/socom2_ghidra_r0004.csv` carrying his label; "agreeing" means our sidecar row
(`recomp/socom2_names_r0004.csv`) names the same function (76 network rows in all, 69 with the identical name).

## Where his names meet our runtime anchors

Our runtime binds online behaviour by address, not by name (research/61): `socom2_addresses.h` and
`socom2_net_bounds.h` (under `third_party/ps2recomp/ps2xRuntime/include/runtime/`) hold per-revision addresses found
by twin reads. His labels land on those addresses independently and say the same thing. This is the strongest
corroboration h62 offers, because neither side used the other.

| our anchor (header member) | r0004 | r0001 | our reading | his label | verdict |
|---|---|---|---|---|---|
| `rtDispatch` (net_bounds) | `0x0063dea0` | `0x006364c0` | the RT message dispatcher: (conn, peer, type byte, body, length) | `rt_msg_client_process_message` | agrees; match.json has no twin here, our header supplies it |
| the frame loop that calls it (net_bounds comment) | `0x0063dcd0` | `0x006362f0` relinked-body | the RT client's per-frame loop; its calls at `+0x330`/`+0x350` are the server memory handlers | `FNC_SortPacketData` | same function, different reading (ours from its calls) |
| `serverMemRead` (addresses) | `0x0063ef08` | `0x00637510` | the server's memory-read request handler (the Medusa class) | `rt_msg_client_process_anti_cheat` | same body, read from the other side: the server's anti-cheat reads client memory through it |
| `serverMemWrite` (addresses) | `0x0063f788` | `0x00637900` | the server's memory-write handler | `rt_msg_client_process_memory_poke` | same body, same reading |
| `gameChat` (net_bounds) | `0x002bc5e0` | `0x002ba9d0` | the game's chat packet handler, lobby and round | note "chat packet receive" | agrees |
| sender-name lookup (net_bounds comment) | `0x002c56e0` | `0x002c3a70` exact | the call the chat handler makes for the sender's name | `LoadPlayerNameFromID` | agrees, and names it |
| `playerNames` (net_bounds) | `0x0044dee4` | `0x004414c4` | 24 slots x 8 B, a slot's first word the player record, name at `+0xe` | note "persona 0 ptr" | agrees (slot 0's record pointer) |
| `chatFanoutRecv` (addresses) | `0x00312100` | `0x002f4ef0` | the Medius chat fan-out receive, formats `"%s: %s"` | note "whisper receiver" | agrees; his note says which chat it serves |
| `roundState` (net_bounds) | `0x004446f8` | `0x00437ce8` | the round record; byte `+0x113` is 3 in a round | object `GameObject` | same static; see [objects.md](objects.md) for the byte disagreement |
| `animTable` (net_bounds) | `0x00441570` | `0x00414bb0` | the animation holder the anim packet handler bounds against | `ZAnim` | agrees |
| `dnasCheck` (addresses) | `0x002cf330` | `0x002cc670` | the DNAS check the capsule stubs | note "DNAS scan" | agrees |
| `netbExUdpRecv` / `netbExUdpSend` | `0x00247d30` / `0x00247fe8` | identical | libnetb UDP receive / send | `libnetb_recv_from` / `libnetb_send_to` | aliases; ours stand |
| `netbExTcpRecv` | `0x002474f8` | identical | libnetb TCP receive (hooked by the runtime) | note "packet sender?" | his question mark; ours is hand-proven by the hook |
| `netbExDescriptorDma` | `0x00247c98` | identical | the descriptor DMA | `libnetb_trans_data` | agrees in substance |

## The RT message client: what is new

His 29 new names here complete a family whose other half we already held from the SOCOM 1 demo's symbols. The ones
that matter for work (full list in [names.md](names.md)):

| r0004 | name | r0001 twin (how) | why it matters |
|---|---|---|---|
| `0x0063dea0` | `rt_msg_client_process_message` | none in match.json; our header: `0x006364c0` | our `rtDispatch`: every incoming RT message passes here |
| `0x0063ea48` | `rt_msg_client_process_app_message` | `0x00637050` exact | where an application (game) message leaves the RT layer: the start of the path to the chat and game-packet handlers (issue #26) |
| `0x0063eb48` | `rt_msg_client_process_disconnect_message` | `0x00637150` relinked-body | the disconnect path |
| `0x0063da20` | `rt_msg_client_store_msg_in_send_buf` | `0x00636040` relinked-body | outgoing buffering |
| `0x0063fdf0` | `rt_msg_client_is_valid_outgoing_msg` | `0x00637f68` exact | the client's own outgoing check |
| `0x0063fea8` | `rt_msg_client_change_state` | `0x00638020` exact | the connection state machine |
| `0x0063a158` | `rt_msg_client_startup` | none | r0004-only placement |
| `0x00640a68`..`0x00640bc0` | `rt_msg_set_2byte` / `_u2byte` / `_4byte` / `_u4byte` | 2 exact, 2 none | the header field writers |
| `0x006365c8` / `0x006365e0` | `rt_comm_platform_receive` / `_receive_from` | seed+delta / exact | the platform receive under libnetb |
| `0x00634de8` | `rt_comm_get_host_by_name` | `0x0062d328` seed+delta | name resolution (what our launcher's DNS answers) |
| `0x00645160`..`0x00645da8` | `rt_time_get_local`, `_get_time_stamp`, `_elapsed`, `_sleep_usecs`, `_platform_init`, `_platform_update_base` | mostly seed+delta | the RT clock; one (`rt_time_platform_init`) we already named on r0001 |

## The game's net layer (`NetI*`) and its packet handlers

| r0004 | name | r0001 twin (how) | note |
|---|---|---|---|
| `0x0061df60` | `NetIInitialize` | -- | ours `NetInitialize` (`string-set`); his is the internal-entry spelling, ours stands |
| `0x0061cdd8` | `NetSendMessageByUDP` | `0x00615660` relinked-body | sibling of our `NetSendMessageByTCP` `0x0061cf58` and `NetSendMessageByUDPToIP` `0x0061ccd0` (his note there: "packet sender broadcast") |
| `0x0061d070` | `NetISendMessage` | `0x006158f8` relinked-body | the common send entry |
| `0x0061a118` | `NetFreeClientPointers` | `0x006129a0` exact | |
| `0x00620858`..`0x00620d48` | `FindFreePingSlot`, `TimeBasePingCallback`, `PING`, `NetIPing`, `NetPingIP` | exact / relinked-body | the ping subsystem |
| `0x00623cd0` | `NetGetClientPingInfo` | `0x0061c510` exact | |
| `0x00621be8` | `NetTickWaitTime` | none | |
| `0x00623470` | `NetUpdateAll` | `0x0061bc68` relinked-body | the per-frame net update |
| `0x00628618` / `0x0062bef8` | `NetRegisterObjectStart` / `NetRegisterMessage` | relinked-body | registration of replicated object classes and message types; our `objectUpdate` handler (`0x00626190`) is registered from `0x006274b8`, in the same neighbourhood |
| `0x00625568` | `ClientConnectsPacketHandler` | (ours, agreeing) | |
| `0x00625a00` | `AcceptClientPacketHandler` | `0x0061e128` relinked-body | |
| `0x00626ce8` | `ArbitrateJoinPacketHandler` | none | r0004-only |
| `0x00627168` | `StreamMediaClientInfoPacketHandler` | none | r0004-only |
| `0x006256f0` / `0x006206d8` | `ClientLeaves_Receiver` / `ClientLeaves_sender` | exact / relinked-body | |
| `0x00621e88` | `UpdateField_Sender` | none | field-update send |
| `0x0061e6d8` | `MapMsgClientFailureReason` | none | |
| `0x0063dcd0` | `FNC_SortPacketData` | `0x006362f0` relinked-body | our frame-loop reading, above |

**The transport's packet-type words.** His labels name 21 words at `0x00686248`..`0x00686298` as the transport's
packet types, `ServerResponse` through `StreamMediaClientInfo` (ping, ownership grant/request/release, arbitrate
join, server version, fragment, field update, object update, data-stream update/end, accept client, delete object,
client update/connect/leaves, request servers, update client status, migrate order). Our census: **all 21 are
referenced by r0004 code** (1-4 `lui`/low-half sites each), so this table is real SOCOM II data. Their values are zero
in his image (taken outside an online session), so the type numbers themselves are still to be read from the code
that fills them -- the first job for a protocol map (research/89's P1).

## Medius, keys and hashes

| r0004 | name | r0001 twin (how) |
|---|---|---|
| `0x00317f00` | `MediusUpdateAccountStats` | none (his in-body notes walk the stats fields: hits, rating, clan name, hometown, description) |
| `0x003f8e50` | `MediusGetNatServiceAddress` | `0x00646a78` exact |
| `0x003fb9b8` | `MediusGetPolicy` | none |
| `0x003fa688` | `UpdateStatsRequest` | none |
| `0x003fa990` | `CreateGameRequest` | `0x006485b8` relinked-body |
| `0x003f97c8` | `NetGetBuildTimeStamp` | `0x006473f0` exact -- **we hold this name as wrong** (`recomp/socom2_name_holds.csv`, S12-R9: the body is `MediusGetBuildTimeStamp`); his label repeats the demo's pairing we rejected. Ours stands |
| `0x00631508` / `0x00631568` / `0x006369f0` | `KM_MD5` / `KM_Salted_MD5` / `KM_Salted_SHA1` | exact |
| `0x00636870` / `0x00636e70` | `AppendKeyChain` / `KM_SetLocalClientID` | exact |
| `0x00631638` / `0x00631798` / `0x00664fd0` | `md5Update` / `md5Final` / `md5Encode` | exact (our `sceEENetMD5Init` `0x006315f8` is his `md5Init`) |
| `0x00654e80` / `0x00654ec0` | `pubKey.0` / `priKey.1` | none -- data words (the RT layer's built-in key pair), referenced by code |
| `0x0030ea90` | `_roomplayer_sort_aux` (over `MediusLobbyWorldPlayerListResponse`) | `0x002f1950` exact |

Two disagreements sit here (both in [names.md](names.md)): `0x003f93c8` his `CreateGameOnSelfRequest` against our
`MediusCheckRequestQueue` (ours `callgraph`, inferred), and `0x00305b70` his `PlayerInfo` against our `cbGetMyClans`
(ours `string-set`). Both are his purpose readings of a body we named by evidence; ours stand.

## The game's own packet code (Zipper's classes)

Mangled names, so the identities come from Zipper's own symbols as they survive in the SOCOM 1 demo (see
[caveats.md](caveats.md): 187 of the 194 new names that carry a mangling exist in the demo's symbol table). He placed them on SOCOM
II's bodies; the placement is his.

| r0004 | name (our form) | r0001 twin (how) |
|---|---|---|
| `0x002ab9a0` / `0x002ab990` | `CMission_CreateNetGame` / `CMission_DestroyNetGame` | `0x002a9f00` / `0x002a9ef0` exact |
| `0x002a5af0` | `CMission_RecoverFromNetError` | none |
| `0x002befc0` | `CZSealState_RemoveNetSeal` | none |
| `0x002c0bc0` | `CZSealState_RemoteSealDeletionCallback` | none |
| `0x002c4200` | `CZBombState_RemoteBombCreationCallback` | none |
| `0x0032a3b0` | `CZNetwork_zNetRegisterRemoteObjectCallback` | none |
| `0x0032aa40` | `CZNetwork_zNetGetValidGameClientCount` | `0x0030d510` exact |
| `0x00315230` | `CZOnlineLobby_SetConnectionInfo` | `0x002f7fa0` relinked-body |
| `0x003182f0` | `CZOnlineLobby_PasswordSecurity` | `0x002faf50` exact |

Agreeing with ours: `CZNetwork_zNetSendApplicationMessage` `0x0032a420` (his note: "packet sender 1") and
`CZNetwork_zNetRemoteObjectUpdateCallback` `0x0032a320`.

**The replication vocabulary.** His labels also carry SOCOM 1's static field-ID names for the replicated classes:
`CZSealState` (position X/Y/Z, yaw, throttles, weapon/stance/peek/action state, six body-part healths, killer index,
fire mode, weapon select, team, AI state, unit, aim elevation, trigger count, vehicle, type, weapons, round and game
stats), `CZGameState` (world string, max game time, initial clock, current time, version, flags, round winner, two
team scores, round index, round and late-join blocks), `CZGrenadeState`, `CZBombState` (position, state, holder,
defuser) and `CZPersonaState` (major/minor persona state, team, name, five "soup" fields, own and server IP, team
counts). In his project they sit at the **demo's** addresses (`0x004759d4`..`0x00475aec`), which SOCOM II code never
references (our census: 0 of 71); see [caveats.md](caveats.md). As a list of what each class replicates the names are
a good guide -- `CZSealState_IncrementalUpdateRemoteSeal` and the 0x5c-byte replication record (research/89 Q3) are
where to look for their SOCOM II counterparts -- but no SOCOM II address comes with them.

## Session logic: his readings

Names he gave from reading the bodies (no symbol source); each is a believed reading of what the function does.

| r0004 | name | r0001 twin (how) |
|---|---|---|
| `0x002bbd50` | `SendVotePacket` | `0x002ba140` relinked-body |
| `0x002bb7a0` | `SpectatorVoteReceiver` | `0x002b9bd0` relinked-body |
| `0x002bbc10` | `ClientReceiveVote` | none |
| `0x002c4bc0` / `0x002c5590` / `0x002c4b60` | `StoreIncomingVote` / `ResolveVoteCount` / `ResolveSpectatorVotes` | exact |
| `0x002c5690` | `LoadPersonaFromIDUpdateVoteCount` | `0x002c3a20` exact |
| `0x002c51c0` | `CheckBanPlayer` | `0x002c3550` relinked-body |
| `0x002ccea0` | `WhisperCheck` | `0x002c9b80` exact |
| `0x0032ad00` / `0x002b9f70` | `DetermineHost` / `FindHost` | none |
| `0x002ae5e0` | `SendMapNameToServer` | none |
| `0x002c4c10` / `0x002c4c40` / `0x002c4d20` | `CheckIfSpectator` / `LoadTeamID` / `GetTeamID` | exact / exact / none |
| `0x002bf060` | `GetSealObjFromID` | `0x002bd420` exact |
| `0x002d19c0` | `UpdateStatsFields` | none (his notes list the player stats offsets it copies; [objects.md](objects.md)) |
| `0x0031dc70` | `IsPlayerIgnored` | `0x003008e0` relinked-body |
| `0x002a1110` / `0x002a0ef0` / `0x002a11b0` | `addPlayer` / `addPlayer_2` / `removePlayer` | relinked-body / relinked-body / exact |

## The online data he annotated

Beyond function names he annotated the online state in memory. Of the 874 data labels that are not SOCOM 1 imports,
265 are referenced by r0004 code (our census). The useful groups:

- **The lobby and game objects**: `0x00452730` (210 code references) labelled as the lobby / game holder; the words at
  `+0xc4`/`+0xc8` "current room + 4" / "current game + 4".
- **The account and stats block** at `0x00669e70`..`0x0066a1cf`: account ID, name, briefing-room ID, the player's
  address, the game-room ID, and the stats record (rating, ladder position, time, rounds won/lost, kills, games,
  hostages and VIPs rescued, deaths, suicides, breaches, headshots, clan tag and name, hometown, accuracy, assists,
  description). Of his 54 labels from `0x00669e70` to `0x0066c93c` (this block and the two below), 29 are referenced.
  A guide for the Medius stats our server returns.
- **The connection state**: `gGameNetConnectionInfo` `0x0066a270` (the `zNetMedius` connection info), the client-ping
  table pointer `0x0066c538`, client ID, host ID and server address words at `0x0066c928`..`0x0066c93c`, and the
  LAN/online IP-stack words at `0x0066c528`/`0x0066c530`.
- **The map selection**: the map count and map-list pointer at `0x0044e074`/`0x0044e078` (both referenced), and
  "map ID for broadcast" / "current map ID" words around `0x0066b752`/`0x0066bc8e`. In his image these hold his
  custom-maps code's values, not the game's defaults ([caveats.md](caveats.md)).

## What this unblocks

- **Issue #26 (which function takes a chat line off the wire):** the path now has names end to end --
  `rt_msg_client_process_message` `0x0063dea0` -> `rt_msg_client_process_app_message` `0x0063ea48` -> the game packet
  dispatch -> `gameChat` `0x002bc5e0` -> `LoadPlayerNameFromID` `0x002c56e0`; the Medius side through
  `chatFanoutRecv` `0x00312100`. One call trace on those five confirms the order.
- **The Medusa hardening** (memory read/write from the server): both handlers are now named the same on both sides;
  his `rt_msg_client_is_valid_outgoing_msg` and `_store_msg_in_send_buf` are the neighbours to read next.
- **His public write-ups on the chat path** are input to the hardening; the hardening's details are kept out of the
  public tree, so this set names only the functions, not the attack.
- **A protocol map:** the 21 transport packet types, the registration calls and the replication vocabulary give a
  table of contents for research/89's P1.
