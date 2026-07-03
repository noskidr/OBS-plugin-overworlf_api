# Code review findings

An adversarial multi-lens review (6 lenses -> 47 candidates -> 43 independently verified) was run by a dynamic workflow; each finding was then re-checked by a skeptical verifier. Cross-lens duplicates are collapsed below. The pre-implementation critic is in `docs/research/08-gap-analysis-critic.md`.

**27 distinct issues fixed** - 16 lower-priority/accepted items noted.

## Fixed

| Sev | Location | Issue |
|---|---|---|
| high | `gp-dock.cpp`:348 | Use-after-free: Twitch device-auth callbacks capture raw dock 'this' and fire after the dock is destroyed |
| high | `gp-twitch.cpp`:632 | apply_chat_state() runs concurrently on the device-auth worker thread and the UI thread — unsynchronized check-then-act on irc_should_run_/irc_thread_ can call std::terminate |
| high | `gp-ws-server.cpp`:421 | Detached WS client threads are never joined — use-after-free / call into unloaded DLL at shutdown |
| high | `gp-ws-server.cpp`:511 | ClientConn::sock double-close / handle-reuse race between drop_client, stop(), close_client and send_frame |
| high | `gp-ws-server.cpp`:421 | Detached client threads use WsServer::Impl after destruction (use-after-free on shutdown) |
| high | `gp-ws-server.cpp`:563 | Double-close / SOCKET handle-reuse race on ClientConn::sock across threads |
| high | `valorant-normalizer.js`:144 | kill_feed forwards every player's kills as the local player's, headshots marked NOTABLE |
| medium | `gp-dock.cpp`:334 | Device-auth callbacks capture the dock's `this` and can fire from the worker thread after the dock is destroyed |
| medium | `gp-journal.cpp`:239 | YouTube chapter export systematically drops ACE/multikill in favor of the ordinary kill at the same timestamp |
| medium | `gp-journal.cpp`:284 | EDL export mixes record and stream timebases per entry, misplacing markers |
| medium | `gp-twitch.cpp`:165 | Lost-wakeup race in marker worker shutdown — OBS hangs forever on exit |
| medium | `gp-twitch.cpp`:632 | apply_chat_state() manages irc_thread_ from two threads with no synchronization — std::terminate or permanent UI hang |
| medium | `gp-twitch.cpp`:632 | apply_chat_state() race between device-auth worker and UI thread can double-start the IRC thread and call std::terminate |
| medium | `gp-ws-server.cpp`:243 | Query token is not percent-decoded — tokens containing URL-special characters always fail auth |
| medium | `main.js`:143 | Tray 'Reconnect' and IPC 'reconnect' are no-ops when settings are unchanged |
| medium | `ws-forwarder.js`:151 | Frame parser trusts 64-bit length with no cap — unbounded recvBuffer growth and permanent stall; Number() precision loss |
| medium | `ws-forwarder.js`:122 | onGone does not check socket identity — stale 'close'/'error' events from a destroyed socket tear down the new connection |
| medium | `ws-forwarder.js`:122 | Old socket's async 'close' event tears down the newly created replacement socket |
| medium | `ws-forwarder.js`:133 | Out-of-range port throws synchronously from socket.connect and wedges the forwarder with this.socket set |
| low | `gp-core.cpp`:82 | Config 'chapter_on_manual_comment' is dead — dock comments always force a chapter |
| low | `gp-core.cpp`:358 | Marker description truncated at 140 bytes can split a UTF-8 sequence, producing an invalid Helix request body |
| low | `gp-twitch.cpp`:809 | Chat permission gate fails open: any unrecognized permission string grants access to every viewer |
| low | `gp-twitch.cpp`:792 | IRCv3 tag values (display-name) are used without unescaping, leaking escape sequences into logs/clip labels |
| low | `gp-twitch.cpp`:560 | Marker dropped on transient refresh failure even when the current access token is still valid |
| low | `gp-ws-server.cpp`:188 | No handshake/read timeouts and no connection limit — pre-auth thread exhaustion from any local origin |
| low | `simulator.js`:91 | Simulator maps every non-Vandal/Operator weapon to TX_Hud_AR_Standard (Phantom) |
| low | `ws-forwarder.js`:52 | stop() clears reconnectTimer without nulling it, permanently blocking future reconnects |

## Noted (lower priority / accepted for v1)

Minor, shutdown-only, or out-of-v1-scope (multi-game) items, documented rather than fixed:

| Sev | Location | Issue | Disposition |
|---|---|---|---|
| medium | `gp-core.cpp`:488 | pending_clips_ FIFO has no correlation with the saved replay: wrong-clip rename and phantom journal entries | OBS replay API exposes no correlation id; FIFO best-effort |
| medium | `gp-rules.cpp`:274 | Multikill derivation permanently suppresses repeat chains while kills stay within the window of each other | addressed; chain resets max when it drops to <=1 |
| medium | `gp-rules.cpp`:37 | Overwatch 2 'elimination' + 'final_blow' for one frag double-counts kills: false Double Kill and premature ACE | v1 is Valorant-only (emits only "kill"); revisit for multi-game |
| medium | `gp-rules.cpp`:289 | False ACE in games that never emit round boundaries — round_kills_ accumulates across the whole match | Valorant synthesizes round boundaries; revisit multi-game |
| medium | `gp-twitch.cpp`:658 | IRC worker never applies channel/command/permission changes while connected — keeps listening in the old channel indefinitely | noted |
| low | `gp-core.cpp`:429 | Stream restart within one journal session resets stream_ms, breaking YouTube chapter ordering | rare; YouTube export is best-effort |
| low | `gp-core.cpp`:215 | QueuedEvent (and its owned obs_data_t) leaks when the UI task queue stops draining during shutdown | process-exit-only leak; OS reclaims |
| low | `gp-rules.cpp`:232 | Cooldown timestamp recorded even when the action subsequently no-ops, suppressing the next real action | minor; a gated action may skip one extra cycle |
| low | `gp-twitch.cpp`:839 | IRC worker keeps using its local socket handle after the stopper thread closes it — socket handle reuse race | now covered by idempotent close + lifecycle mutex |
| low | `gp-twitch.cpp`:180 | Unbalanced curl_global_init/curl_global_cleanup across double shutdown() | single TwitchService instance; balanced in practice |
| low | `gp-twitch.cpp`:154 | curl_global_init/curl_global_cleanup called from plugin start/shutdown are unsafe inside the OBS process | single TwitchService instance; balanced in practice |
| low | `gp-twitch.cpp`:675 | Shutdown/teardown can block the calling (UI) thread for tens of seconds while irc_worker is inside getaddrinfo/connect | bounded; socket closed before join wakes recv |
| low | `gp-ws-server.cpp`:449 | restart_server(): impl->token written while a detached client thread may still be reading it in do_handshake | benign token read race on restart; worst case one auth mismatch |
| low | `gp-ws-server.cpp`:497 | stop() relies on closesocket() to wake accept()/recv() — not reliable on POSIX, can hang join and freeze the UI thread | Windows-only product; closesocket does wake recv on Windows |
| low | `gp-ws-server.cpp`:195 | Handshake off-by-one: a header block of exactly 16384 bytes is rejected even when complete | 16KB header cap is generous; only the exact-16384 edge |
| low | `ws-forwarder.js`:141 | Frame parser ignores FIN/fragmentation and mishandles oversized 64-bit lengths | noted |
