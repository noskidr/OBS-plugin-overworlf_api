# Streaming Platform Integrations for Automated Highlight Marking â€” Verified Research Report

All claims below were verified 2026-07-03 against official documentation (fetched raw HTML from dev.twitch.tv where noted, so quotes are verbatim), official GitHub sources, or a live protocol test performed during this research session.

---

## 1. Twitch Helix â€” Create Stream Marker

**Endpoint:** `POST https://api.twitch.tv/helix/streams/markers`
Verified verbatim from the raw HTML of the Helix reference (`id="create-stream-marker"` section).

**Doc description (verbatim):** "Adds a marker to a live stream. A marker is an arbitrary point in a live stream that the broadcaster or editor wants to mark, so they can return to that spot later to create video highlights."

**Hard constraints (verbatim):**
> "You may not add markers:
> - If the stream is not live.
> - If the stream has not enabled video on demand (VOD).
> - If the stream is a rerun of a past broadcast."

**Authorization:** "Requires a user access token that includes the `channel:manage:broadcast` scope."
- CAUTION: several third-party writeups say `user:manage:broadcast` â€” that is WRONG. Verified three ways: raw reference HTML, the official guide page (https://dev.twitch.tv/docs/api/markers/), and Twitch's own mock API (`twitch-cli`, `internal/mock_api/endpoints/streams/markers.go`: `http.MethodPost: {"channel:manage:broadcast"}`).

**Request:** no query parameters; JSON body only:
| Field | Type | Required | Doc text (verbatim) |
|---|---|---|---|
| `user_id` | String | Yes | "The ID of the broadcaster that's streaming content. This ID must match the user ID in the access token or the user in the access token must be one of the broadcaster's editors." |
| `description` | String | No | "A short description of the marker to help the user remember why they marked the location. The maximum length of the description is 140 characters." |

**Example request (verbatim from docs):**
```bash
curl -X POST 'https://api.twitch.tv/helix/streams/markers' \
  -H 'Authorization: Bearer cfabdegwdoklmawdzdo98xt2fo512y' \
  -H 'Client-Id: uo6dggojyb8d6soh92zknwmi5ej1q2' \
  -H 'Content-Type: application/json' \
  -d '{"user_id":"123", "description":"hello, this is a marker!"}'
```

**Example response (verbatim):**
```json
{"data":[{"id":123,"created_at":"2018-08-20T20:10:03Z","description":"hello, this is a marker!","position_seconds":244}]}
```
Response fields: `id` (String â€” note the example shows a bare number; treat as string), `created_at` ("The UTC date and time (in RFC3339 format) of when the user created the marker."), `position_seconds` (Integer â€” "The relative offset (in seconds) of the marker from the beginning of the stream."), `description`.

**Response codes (verbatim, complete):**
- `200 OK` â€” "Successfully created the marker."
- `400` â€” "The user_id field is required." / "The length of the string in the description field is too long."
- `401` â€” "The Authorization header is required and must contain a user access token." / "The user access token must include the channel:manage:broadcast scope." / "The access token is not valid." / "The Client ID specified in the Client-Id header does not match the Client ID specified in the access token."
- `403` â€” "The user in the access token is not authorized to create video markers for the user in the user_id field. The user in the access token must own the video or they must be one of the broadcaster's editors."
- `404` â€” "The user in the user_id field is not streaming live." / "The ID in the user_id field is not valid." / "The user hasn't enabled video on demand (VOD)."

**Precision:** `position_seconds` is integer-second granularity. There is **no position/timestamp request parameter of any kind** â€” the marker is placed server-side at the moment Twitch processes the call (the reference's example request caption reads: "Creates a marker at the current location in user 123's stream."). Backdating is impossible; see section 6.

**Rate limits:** No marker-specific limit is documented. Global Helix limiting is a token-bucket: "Your app is given a bucket for app access requests and a bucket for user access requests" and for user tokens "the limits are applied per client ID per user per minute." Track headers `Ratelimit-Limit` / `Ratelimit-Remaining` / `Ratelimit-Reset` (Unix epoch); documented example shows `Ratelimit-Limit: 800`. On 429: "use the Ratelimit-Reset header to learn how long you must wait." Docs also warn: "Some API endpoints may return HTTP 429 response codes for reasons unrelated to the general rate limit bucket." A marker per game event (kills etc.) is far below this budget.

### Companion endpoint â€” Get Stream Markers (for exports/verification)
`GET https://api.twitch.tv/helix/streams/markers` â€” "Requires a user access token that includes the `user:read:broadcast` **or** `channel:manage:broadcast` scope." Query params: `user_id` (markers from that user's most recent video) or `video_id` (mutually exclusive; 400 if both/neither), `first` (1â€“100, default 20), `before`/`after` cursors. Response markers include `id`, `created_at`, `description`, `position_seconds`, and `url` â€” "A URL that opens the video in Twitch Highlighter", e.g. `"https://twitch.tv/twitchname/manager/highlighter/456?t=0h4m06s"`. 404 if "The user specified in the user_id query parameter doesn't have videos."

### Bonus (relevant to auto-clip): Create Clip and NEW Create Clip From VOD
- **Create Clip:** `POST https://api.twitch.tv/helix/clips?broadcaster_id={id}` â€” scope `clips:edit`. "This API captures up to 90 seconds of the broadcaster's stream. The 90 seconds spans the point in the stream from when you called the API." "By default, Twitch publishes up to the last 30 seconds of the 90 seconds window." Optional query params now include `title` (String) and `duration` (Float, "range from 5 to 60 inclusively with a precision of 0.1. The default is 30.") â€” the old `has_delay` param is gone from the current reference. Returns `202 Accepted` with `data:[{id, edit_url}]`; "Creating a clip is an asynchronous process" â€” poll Get Clips. `edit_url` "is valid for up to 24 hours or until the clip is published, whichever comes first." 404: "The broadcaster in the broadcaster_id query parameter must be broadcasting live." 403 covers follower/sub-only clip restrictions and bans. 400 includes "The category is not clippable." and "The title did not pass AutoMod checks."
- **Create Clip From VOD (marked "NEW" in the reference):** `POST https://api.twitch.tv/helix/videos/clips` â€” "Creates a clip from a broadcaster's VOD on behalf of the broadcaster or an editor of the channel. Since a live stream is actively creating a VOD, this endpoint can also be used to create a clip from earlier in the current stream." Auth: "Requires an app access token or user access token that includes the `editor:manage:clips` or `channel:manage:clips` scope." Query params: `editor_id` (required, must match token user), `broadcaster_id` (required), `vod_id` (required), `vod_offset` (required, Integer â€” "the clip will start at (vod_offset - duration) and end at vod_offset"), `duration` (Float 5â€“60, default 30), `title` (required). This is the **only Twitch API that lets you clip a moment retroactively at an exact offset** â€” ideal for game-event-driven clipping (mark event time, clip `vod_offset = event_offset + a few seconds`).

---

## 2. Twitch OAuth Device Code Grant (native desktop app, no backend)

Verified verbatim from raw HTML of https://dev.twitch.tv/docs/authentication/getting-tokens-oauth/ (Device code grant flow section).

**Availability / client types (verbatim):** "Device Code Grant Flow (DCF) allows for public and confidential client types." Public clients:
> "- Do not need to maintain a client secret.
> - Can refresh an access token without passing a client secret.
> - Are only limited to the usage of device authorization grant flow to obtain OAuth tokens and cannot use any of the other flows like client credentials, implicit grant flow."

And the platform recommendation, directly applicable to this project: "Generally, if you device is secure, then confidential is the client type you should use, however if your application is on a more open platform (such as windows) we suggest using public."

**Registration requirements:** Register at the dev console (requires 2FA: users "must also enable two-factor authentication (2FA) for [their] account"). Fields: unique Name, OAuth Redirect URLs (required field but unused by DCF â€” `http://localhost` is fine; docs' own examples use `http://localhost:3000`), Category, and the Client Type selector (Public/Confidential; existing apps can be switched, per the developer console and forum guidance). For DCF with a Public client you ship **client_id only â€” no secret**. Note Twitch has **no PKCE support** on the authorization-code flow (no mention anywhere in the auth docs), which is exactly why DCF + public client is the sanctioned native-app path.

**Step 1 â€” start flow:** `POST https://id.twitch.tv/oauth2/device` (form-encoded). Params: `client_id` (Yes), `scopes` (Yes â€” note: the parameter is literally named `scopes`, plural, unlike `scope` in the other flows; "A space-delimited list of scopes... You must URL encode the list.").
```bash
curl -X POST --location 'https://id.twitch.tv/oauth2/device' \
  --form 'client_id="<clientID>"' --form 'scopes="<scopes>"'
```
Response (verbatim example): `device_code` ("The identifier for a given user."), `expires_in` (e.g. `1800` = 30 min), `interval` (e.g. `5` seconds), `user_code` (e.g. `"ABCDEFGH"`), `verification_uri` â€” e.g. `"https://www.twitch.tv/activate?public=true&device-code=ABCDEFGH"` (open this in the user's browser; the code is pre-filled via query param).

**Step 2 â€” poll for token:** `POST https://id.twitch.tv/oauth2/token` (form-encoded) with `client_id`, `scopes`, `device_code`, `grant_type=urn:ietf:params:oauth:grant-type:device_code`. Poll no faster than `interval`. Documented poll-time errors: before authorization â†’ `{"status":400,"message":"authorization_pending"}`; device_code reuse ("The device_code is one time use only") â†’ `{"status":400,"message":"invalid device code"}`. Twitch does **not** document RFC 8628's `slow_down`/`expired_token` responses â€” handle generic 400s defensively and restart the flow after `expires_in`.

**Success response:** `{access_token, expires_in, refresh_token, scope (array), token_type:"bearer"}`.

**Token lifetimes (verbatim):** "All access tokens obtained have the same expiry like before, i.e. 4 hours." DCF refresh tokens: "These tokens are for one time use only, meaning if they are used in refreshing a token they will become invalid after use" (rotation â€” always persist the newly returned refresh_token) and "There is an expiry on the refresh token which is inactive, which is set to 30 days. After a refresh token expires the user is expected to start the DCF flow once again." (The Refresh Tokens page words it as: "refresh tokens generated by a Public client type will expire 30 days after they are generated" â€” plan for re-auth after 30 days either way.) Reusing a spent refresh token â†’ `{"status":400,"message":"Invalid refresh token"}`.

**Refresh call (public client, no secret):** `POST https://id.twitch.tv/oauth2/token` with `grant_type=refresh_token&refresh_token=<url-encoded>&client_id=<id>`; the Refresh Tokens doc states of `client_secret`: "This field is not required if your application's client type was set to public."

**Mandatory hygiene:** "Third-party apps that call the Twitch APIs and maintain an OAuth session **must** call the `/validate` endpoint to verify that the access token is still valid" (`GET https://id.twitch.tv/oauth2/validate`; Twitch elsewhere specifies hourly validation for long-lived sessions).

**Scopes to request for this project:** `channel:manage:broadcast` (markers) + `channel:read:redemptions` (channel point events) + `clips:edit` (Create Clip) [+ `channel:manage:clips` if using Create Clip From VOD; + `user:read:chat` if reading chat via EventSub instead of anonymous IRC; + `user:write:chat` to post !clip confirmations via Send Chat Message].

---

## 3. Twitch IRC anonymous read-only (justinfan)

**Official endpoints (from https://dev.twitch.tv/docs/chat/irc/):** WebSocket `wss://irc-ws.chat.twitch.tv:443` (non-TLS WebSocket was decommissioned); IRC `irc.chat.twitch.tv:6697` (TLS) / `:6667` (plain). Capabilities: `CAP REQ :twitch.tv/membership twitch.tv/tags twitch.tv/commands`.

**Anonymous status:** `justinfan<digits>` has never been officially documented ("Justinfan was never officially documented" â€” Twitch staff on the developer forums), but it remains fully functional. **Live-verified during this research (2026-07-03)** against `irc.chat.twitch.tv:6697`:
- Sent only `CAP REQ :twitch.tv/tags twitch.tv/commands` + `NICK justinfan73824` (no PASS) â†’ received `CAP * ACK` and `001 justinfan73824 :Welcome, GLHF!`.
- `JOIN` succeeded; received tagged `ROOMSTATE`.
- Received live `PRIVMSG` lines with **full tag sets**, e.g.:
  `@badge-info=;badges=;client-nonce=...;color=;display-name=TowerDee;emotes=;first-msg=0;id=0436ea0f-...;mod=0;returning-chatter=0;room-id=552120296;subscriber=0;tmi-sent-ts=1783106877071;turbo=0;user-id=156680136;user-type= :towerdee!... PRIVMSG #zackrawrr :LOL o7`

So an anonymous connection CAN read chat with tags: `badges` (includes `broadcaster/1`, `moderator/1`, `subscriber/N`, `vip/1`), `badge-info` (sub tenure), `mod`, `subscriber`, `user-id`, `display-name`, `tmi-sent-ts` â€” everything needed to permission-gate a `!clip` command (gate on `badges` containing `broadcaster` or `mod=1`; optionally `subscriber=1`/`vip`). Anonymous connections are read-only (PRIVMSG sends are ignored/disconnected), which is fine: send the confirmation via Helix Send Chat Message (`POST /helix/chat/messages`, scope `user:write:chat`) using the DCF token, or skip confirmations.

**Rate limits (official Chat docs):** JOIN: "20 join attempts per 10 seconds per user"; auth: "20 authentication attempts per 10 seconds per user (IRC only)"; messages: 20 per 30s normal, 100 per 30s as broadcaster/mod/VIP; verified bots 7,500/30s and 2,000 joins/10s.

**2024â€“2026 policy context:** IRC is not deprecated â€” the docs still describe it but steer new work to EventSub: "The preferred method of viewing and sending chats on Twitch is through EventSub and Twitch API, but Twitch historically has an IRC interface." The 2024 "Giving broadcasters control" rollout capped **concurrent channel joins at 100 per user/connection identity** (final limit May 15â€“16, 2024; verified-bot exemption fully removed June 26, 2024). Staff explicitly declined to guarantee justinfan behavior ("it could very well be the case that you wont be able to connect to more than 100 channels using justinfan"). For this project â€” reading ONE channel (the streamer's own) â€” anonymous IRC is comfortably within limits and requires zero extra scopes. Recommended architecture: use justinfan IRC for chat-command input in v1, with the EventSub `channel.chat.message` subscription (scope `user:read:chat`, condition `broadcaster_user_id` + `user_id`) as the already-authenticated fallback since the app will hold a DCF user token anyway.

---

## 4. Twitch EventSub over WebSocket â€” channel point redemptions

**Subscription type (verbatim from subscription-types page):** `channel.channel_points_custom_reward_redemption.add`, version `"1"` â€” "sends a notification when a viewer has redeemed a custom channel points reward on the specified channel." Authorization: "Must have `channel:read:redemptions` or `channel:manage:redemptions` scope." Condition: `broadcaster_user_id` (required); `reward_id` "optional; gets notifications for a specific reward." (Related: `...redemption.update` v1 for status changes; `channel.channel_points_automatic_reward_redemption.add` for built-in rewards; `channel.custom_power_up_redemption.add` for Power-ups.)

**Event payload fields (verbatim from EventSub reference):** `id` (redemption id), `broadcaster_user_id/login/name`, `user_id`, `user_login`, `user_name`, `user_input` ("The user input provided. Empty string if not provided."), `status` ("Defaults to unfulfilled. Possible values are unknown, unfulfilled, fulfilled, and canceled."), `redeemed_at` ("RFC3339 timestamp"), `reward{id, title, cost, prompt}`.

**WebSocket protocol (verbatim from handling-websocket-events):** connect to `wss://eventsub.wss.twitch.tv/ws`, optional `?keepalive_timeout_seconds=` (10â€“600). First message is `session_welcome`:
```json
{"metadata":{"message_id":"...","message_type":"session_welcome","message_timestamp":"..."},
 "payload":{"session":{"id":"AQoQILE98gtqShGmLD7AM6yJThAB","status":"connected","connected_at":"...","keepalive_timeout_seconds":10,"reconnect_url":null}}}
```
"By default, you have 10 seconds from the time you receive the Welcome message to subscribe to an event... If you don't subscribe within this timeframe, the server closes the connection." Other message types: `session_keepalive` (reset your liveness timer; assume dead if nothing arrives within keepalive window), `notification`, `session_reconnect` ("you have 30 seconds to reconnect" to the supplied `reconnect_url`; reconnects don't count against connection limits), `revocation`. Client-to-server messages are forbidden except pongs. "If a WebSocket connection is lost... There is no replay of events."

**Creating the subscription:** `POST https://api.twitch.tv/helix/eventsub/subscriptions` with headers `Authorization: Bearer <user token>`, `Client-Id`, `Content-Type: application/json` and body:
```json
{"type":"channel.channel_points_custom_reward_redemption.add","version":"1",
 "condition":{"broadcaster_user_id":"<broadcaster id>"},
 "transport":{"method":"websocket","session_id":"<id from session_welcome>"}}
```
**Token rule (verbatim):** for WebSocket transport "you must use a user access token only. The request fails if you use an app access token." (Webhooks are the inverse.) A **device-flow token is a user access token, so this works from a desktop app with no server** â€” this is the sanctioned pattern. Because the broadcaster's own DCF token carries `channel:read:redemptions`, the subscription costs 0 ("There is no cost for subscriptions that require a user to authorize your application"); the redemption example itself shows `"cost": 0`. Limits: 3 WebSocket connections with enabled subscriptions per client-id+user, 300 enabled subscriptions per connection, `max_total_cost` 10 for websocket transport. On successful create, `status` is `"enabled"` immediately (no challenge for websockets). Timestamps are RFC3339 with nanoseconds.

**Flow for the companion app:** DCF login â†’ open WS â†’ read `session.id` â†’ create subscriptions (redemptions + optionally `stream.online`/`stream.offline`, `channel.chat.message`) within 10 s â†’ on redemption of a designated "Clip that!" reward, fire marker/replay-buffer actions.

---

## 5. YouTube Live â€” markers/clips/chapters

**No public API for VOD markers or clips â€” CONFIRMED by absence in the official API surface:** the YouTube Data API v3 reference has no `clips` resource and the `videos` resource has **no properties for markers, chapters, or clips** (verified against https://developers.google.com/youtube/v3/docs/videos â€” nothing beyond `snippet`, `contentDetails`, `status`, `liveStreamingDetails`, etc.). The viewer "Clip" feature (youtube.com/clip) has zero API; only scrapers/issue-tracker feature requests exist. The Live Streaming API's only "marker-like" call is:

**`liveBroadcasts.cuepoint` = ads only.** `POST https://www.googleapis.com/youtube/v3/liveBroadcasts/cuepoint` â€” "Inserts a cuepoint into a live broadcast. The cuepoint might trigger an ad break." `cueType` is required and "must be set to `cueTypeAd`" (the enum's only value). Other properties: `durationSecs` (uint, default 30), `insertionOffsetTimeMs` (long, offset from beginning of broadcast; cannot combine with `walltimeMs`), `walltimeMs` (epoch ms). Scopes: `youtube`, `youtube.force-ssl`, `youtubepartner`. Useless for highlights; do not use.

**The only viable YouTube mechanism: description-timestamp chapters on the VOD.** Official rules (support.google.com/youtube/answer/9884579, verbatim): "In the Description, add a list of timestamps and titles." / "Make sure that the first timestamp you list starts with 00:00." / "Your video should have at least three timestamps listed in ascending order." / "The minimum length for video chapters is 10 seconds." Automatic chapters may apply otherwise and are overridden by manual timestamps; ineligible if "the channel has any active strikes, or if the content may be inappropriate to some viewers."

**Programmatic write path:** `videos.update` (`PUT https://www.googleapis.com/youtube/v3/videos?part=snippet`), quota cost **50 units** (default project quota: "100 search.list calls, 100 videos.insert calls, and 10,000 units per day combined for all other endpoints"). Critical pitfalls (verbatim): "If you are submitting an update request, and your request does not specify a value for a property that already has a value, the property's existing value will be deleted." And `snippet.title` / `snippet.categoryId` are required "if the request updates the video resource's snippet." So: `videos.list part=snippet` first, append the chapter block to the existing description, resend title+categoryId+description together. `snippet.description`: "The property value has a maximum length of 5000 bytes and may contain all valid UTF-8 characters except < and >." (bytes, not chars â€” budget ~60â€“100 chapter lines).

**Offset source:** `videos.list part=liveStreamingDetails` on the live video ID â†’ `liveStreamingDetails.actualStartTime` ("The time that the broadcast actually started. The value is specified in ISO 8601 format."), plus `actualEndTime`, `activeLiveChatId`, `concurrentViewers`. OAuth scope `https://www.googleapis.com/auth/youtube.force-ssl` (or `youtube`) is enough for list+update; Google supports proper OAuth **installed-app / loopback** flows (and its own device flow, though YouTube scopes on the device flow are restricted to TVs â€” use loopback `http://127.0.0.1:<port>` for a desktop app).

---

## 6. Timestamp math

**Twitch â€” confirmed: no timestamp parameter.** Create Stream Marker's body is exactly `user_id` + `description`; the marker lands at the live position when Twitch processes the request ("Creates a marker at the current location in..."), returned as integer `position_seconds` "from the beginning of the stream." Consequences:
- The VOD timeline equals the ingest timeline, so a marker fired the instant a game event happens lands at the right VOD spot regardless of viewer-side latency or broadcaster-configured stream delay (those delay playback, not ingest).
- OBS-side render/stream delay (e.g., OBS "Stream Delay" of D seconds) DOES shift content: gameplay at wall-time A leaves the encoder at A+D, so the on-screen moment sits ~D seconds LATER in the VOD than the marker fired at A. Slightly-early markers are desirable for highlights; if D is large, add D to your description metadata (you cannot move the marker).
- You cannot backdate ("that kill 20 s ago"). Mitigations: (a) put the elapsed offset in the 140-char description, e.g. `ACE -12s`; (b) use **Create Clip From VOD** with `vod_offset` for exact retroactive capture; (c) plain Create Clip already covers ~the last 30â€“90 s.
- Network/API latency adds ~0.1â€“1 s; `position_seconds` is integer seconds anyway.
- Encoder reconnects: while Twitch holds the session open (~90 s grace), server stream time keeps running and later markers stay VOD-aligned (the VOD just contains the gap). If the session ends and a new stream/VOD starts, `position_seconds` restarts from the new stream's beginning â€” re-detect via `stream.online` EventSub and reset any local clocks.

**YouTube â€” you must track offsets yourself.** `chapter_offset = event_wallclock_UTC âˆ’ liveStreamingDetails.actualStartTime` (both UTC; use NTP-sane local clock). Pitfalls:
- Do NOT count from "when OBS started streaming": YouTube's `actualStartTime` is when YouTube transitioned the broadcast live, which can trail encoder start by seconds (auto-start) or minutes (manual "Go live" in Studio). Always fetch `actualStartTime`.
- The archived VOD normally begins at `actualStartTime`, but trims/processing can shift content by a few seconds; clamp the first computed chapter to `0:00` (required anyway) and round to whole seconds.
- Encoder reconnects: brief dropouts usually stay in one VOD (with frozen/gap segments â€” offsets after the gap remain wall-clock correct because YouTube keeps the timeline running); a long outage or a second `liveBroadcast` produces a separate video with its own `actualStartTime` â€” detect via `liveBroadcasts.list` / status polling and re-anchor.
- Latency mode (normal/low/ultra-low) and DVR affect viewers, not the VOD timeline â€” no compensation needed.
- Enforce chapter validity before writing: first entry `0:00`, â‰¥3 entries, ascending, â‰¥10 s apart; merge events closer than 10 s.
- Stream delay added in the encoder shifts content later relative to wall clock exactly as on Twitch â€” subtract the configured delay from computed offsets if the user runs one.

**Practical design:** the OBS plugin should record for every event: wall-clock UTC, `obs_get_video_frame_time()`-based recording offset, and (when live) seconds since `OBS_FRONTEND_EVENT_STREAMING_STARTED`. That triple lets you emit Twitch markers immediately, YouTube chapters post-stream, and local recording chapter marks without cross-dependency.

---

## 7. Kick.com and other platforms (brief)

**Kick (docs.kick.com):** Official public API exists (launched 2025; OAuth 2.1 at `id.kick.com` â€” `GET /oauth/authorize` + `POST /oauth/token`, PKCE `S256` **required**, but `client_secret` is also required at token exchange, i.e., no public-client story). Endpoints cover categories, users, channels, chat (send/delete), livestreams, moderation, Channel Rewards, KICKs leaderboard, public key. **No clips, markers, highlights, or VOD endpoints exist.** Events (`chat.message.sent`, `livestream.status.updated`, `moderation.banned`, `kicks.gifted`, channel rewards, etc.) are delivered via `POST /public/v1/events/subscriptions` whose transport enum is `["webhook"]` **only** â€” a publicly reachable HTTPS callback is mandatory, which disqualifies a pure desktop app without a relay. Verdict: **skip for v1**; at most offer local-only actions (replay-buffer clip on hotkey) while streaming to Kick. Kick's site has clips, but only via undocumented private endpoints â€” do not build on them.

**Others:** YouTube covered above (chapters only). Facebook Gaming has no marker API. Trovo's API has chat/channel but no marker/clip creation worth v1. Twitch is the only platform with a true live-marker API; the portable cross-platform fallback is the local layer: OBS recording chapter markers (obs_frontend hotkeys / `obs-websocket`), replay buffer saves, and a post-stream chapter/timestamps export file.

---

## Actionable v1 matrix

| Capability | Twitch | YouTube | Kick |
|---|---|---|---|
| Live marker | POST /helix/streams/markers (channel:manage:broadcast) | none (cuepoint=ads only) | none |
| Retroactive clip | POST /helix/videos/clips (vod_offset) or POST /helix/clips | none | none |
| Chat command input | anonymous justinfan IRC (live-verified) or EventSub channel.chat.message | liveChatMessages (quota-heavy; optional later) | webhook-only (skip) |
| Channel-point trigger | EventSub WS channel.channel_points_custom_reward_redemption.add v1 (channel:read:redemptions) | n/a | Channel Rewards API exists but webhook-only |
| Auth w/o backend | Device Code Grant, public client, client_id only, 4 h tokens, rotating refresh (30-day cap) | OAuth installed-app loopback | not viable (secret required) |
| Post-stream markers | Get Stream Markers â†’ export | videos.update description chapters (0:00, â‰¥3, â‰¥10 s) | none |
