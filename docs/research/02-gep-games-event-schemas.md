# Overwolf GEP (Game Events Provider): Game Coverage + Exact Event Schemas

Research date: 2026-07-03. Primary sources: dev.overwolf.com official docs (the new home of overwolf.github.io content), the live `game-events-status.overwolf.com` endpoints (fetched live today), the `@overwolf/ow-electron-packages-types` .d.ts source, and the official `ow-electron-packages-sample` repo. Cross-checked against the local Insights Capture V3 reverse-engineering findings in `G:\personal\insights_capture\notes\REVERSE_ENGINEERING_FINDINGS.md`.

**Docs URL scheme (old overwolf.github.io URLs 404; use these):**
- GEP intro: `https://dev.overwolf.com/ow-native/live-game-data-gep/live-game-data-gep-intro/`
- Per-game pages: `https://dev.overwolf.com/ow-native/live-game-data-gep/supported-games/{valorant | league-of-legends | counter-strike-2 | apex-legends | fortnite | rocket-league | dota-2 | playerunknowns-battlegrounds | ...}/`
- API reference: `https://dev.overwolf.com/ow-native/reference/games/events/`
- Health: `https://dev.overwolf.com/ow-native/live-game-data-gep/game-events-status-health/` and `.../verifying-events-for-your-app/`
- ow-electron GEP: `https://dev.overwolf.com/ow-electron/live-game-data-gep/live-game-data-gep-intro/`
- Game IDs guide: `https://dev.overwolf.com/ow-native/guides/dev-tools/games-ids/` (table is JS-loaded; canonical machine source is the status endpoint below, or local `%localappdata%\Overwolf\...gameList.xml`)

---

## 1. Supported games + numeric game class IDs (all VERIFIED)

Two authoritative machine-readable sources, both fetched today:

**(a) Live list** â€” `https://game-events-status.overwolf.com/gamestatus_prod.json` returns a JSON **array** (~75 entries). Each entry:
```json
{"game_id":21640,"name":"Valorant","state":2,"disabled":false,"disabled_electron":false,"is_vgep":false,"vgep_prefix":null,"min_gep_version":"307.4.6","min_gep_version_electron":"307.4.6"}
```

**(b) ow-electron-supported subset** â€” `const enum kGepSupportedGameIds` in `gep-supported-games.d.ts` of npm package `@overwolf/ow-electron-packages-types` (v1.1.4). All 59 entries verbatim:

| Game | ID | Game | ID |
|---|---|---|---|
| ApexLegends | 21566 | MarvelRivals | 24890 |
| ArcRaiders | 27168 | Minecraft | 8032 |
| ArknightsEF | 27724 | MinecraftBedrock | 22176 |
| BaldursGateIII | 22088 | MHWilds | 25446 |
| BlackMythWukong | 24504 | NewWorld | 21816 |
| CoDGI | 27860 | OnceHuman | 23930 |
| CallofDutyModernWarfareII | 22328 | Overwatch (2) | 10844 |
| CallofDutyVanguard | 21876 | Palworld | 23944 |
| ContentWarning | 24110 | PathofExile | 7212 |
| **CS2** | **22730** | PathofExile2 | 24886 |
| DarkandDarker | 22584 | PEAK | 26092 |
| Deadlock | 24482 | **PUBG** | **10906** |
| DiabloIV | 22700 | REPO | 25448 |
| **Dota2** | **7314** | **Rainbow6Siege** | **10826** |
| ELDENRINGNIGHTREIGN | 25918 | REMATCH | 26120 |
| EscapeFromTarkov | 21634 | Roblox | 4688 |
| FinalFantasyXIVOnline | 6350 | RobloxMicrosoftedition | 22174 |
| **Fortnite** | **21216** | **RocketLeague** | **10798** |
| GenshinImpact | 21656 | ScheduleI | 25610 |
| HearthstoneHeroesofWarcraft | 9898 | SonsoftheForest | 22638 |
| HELLDIVERS2 | 24000 | Splitgate2 | 25884 |
| HonkaiStarRail | 22804 | StreetFighter6 | 22894 |
| **LeagueofLegends** | **5426** | SUPERVIVE | 24346 |
| LeagueofLegendsPBE | 22848 | TeamfightTactics | 21570 |
| LethalCompany | 23522 | TheFinals | 23478 |
| LostArk | 21864 | **VALORANT** | **21640** |
| MagictheGatheringArena | 21308 | Warframe | 8954 |
| | | Warhammer40000SpaceMarine2 | 24548 |
| | | WoW | 765 |
| | | WutheringWaves | 24300 |
| | | XKO (2XKO) | 26840 |

Additional IDs present only in the live gamestatus_prod.json (native Overwolf, not in the electron enum): 10902 LoL Launcher, 21404 Splitgate, 21620 Legends of Runeterra, 21556 LOL Arena (+215561 Arena PBE, 215701 TFT PBE, 13956 LOL Swarm, 21568 LOL Brawl), 26462 Battlefield 6, 27618 Hytale, 27936 Subnautica 2, 27886 Windrose, 27858 Slay the Spire 2, 46882/46883 Roblox Garden/Rivals.

All IDs the task asked to verify: **Valorant=21640, LoL=5426, CS2=22730 (confirmed â€” note the CS2 docs page also mentions Steam AppID 730, which is NOT the Overwolf class id), Apex=21566, Fortnite=21216, Rocket League=10798, Dota 2=7314, Overwatch 2=10844, PUBG=10906, R6 Siege=10826, Marvel Rivals=24890.**

---

## 2. VALORANT (21640) â€” full feature list + every key

Docs: `https://dev.overwolf.com/ow-native/live-game-data-gep/supported-games/valorant/`. Ground truth key list from live `https://game-events-status.overwolf.com/21640_prod.json` (state=2/yellow today; min_gep_version 307.4.6; Insights Capture V3 ships GEP 308.0.4).

**Features:** `gep_internal`, `game_info`, `me`, `kill`, `death`, `match_info`.

### Info updates (type 1 keys)
| Feature | Key | Category | Example value |
|---|---|---|---|
| gep_internal | version_info | gep_internal | `{"local_version":"157.0.1","public_version":"157.0.1","is_updated":true}` (JSON string) |
| game_info | scene | game_info | `"MainMenu"`, `"Triad"` (=Haven), `"Duality"` (=Bind), `"CharacterSelectPersistentLevel"` |
| game_info | state | game_info | `"WaitingToStart"`, `"InProgress"`, `"LeavingMap"`, `"Aborted"`, `"Init"` |
| me | player_name | me | `"Doom#5339"` |
| me | player_id | me | `"f4029eff-92e6-56db-adba-4d073df968a4"` |
| me | region | me | `"esp"` |
| me | server | me | `"eu"` |
| me | agent | me | `"Phoenix_PC_C"` (internal codename) |
| me | health | me | `"100"` |
| me | abilities | me | `"{\"C\":true,\"Q\":true,\"E\":true,\"X\":false}"` (JSON string; key state=3/red today) |
| kill | kills / assists / headshots | kill | running totals `"1"`, `"2"`â€¦ |
| death | deaths | death | `"1"` |
| match_info | pseudo_match_id | match_info | `"0c0ea3df-97ea-4d3a-b1f6-f8e34042251f"` (Overwolf-generated UUID) |
| match_info | match_id | match_info | `"d7817c2f-0456-4732-a023-144d8b345a37"` (Riot match id) |
| match_info | round_number | match_info | `"1"`, `"2"`, â€¦ |
| match_info | round_phase | match_info | `"shopping"`, `"combat"`, `"end"`, `"game_end"` â€” envelope: `{"info":{"match_info":{"round_phase":"shopping"}},"feature":"match_info"}` |
| match_info | team | match_info | `"attack"` / `"defense"` |
| match_info | score | match_info | JSON string `{"won":9,"lost":2}` |
| match_info | match_score | match_info | JSON string `{"team_0":0,"team_1":2}` |
| match_info | match_outcome | match_info | `"victory"`, `"draw"` (docs list these two) |
| match_info | game_mode | match_info | JSON string `{"mode":"bomb","custom":true,"ranked":"0"}`; modes: bomb (unrated/comp), quick bomb (Spike Rush), deathmatch, escalation, swift, range, team deathmatch |
| match_info | map | match_info | internal codenames: Infinity=Abyss, Triad=Haven, Duality=Bind, Bonsai=Split, Ascent=Ascent, Port=Icebox, Foxtrot=Breeze, Canyon=Fracture, Pitt=Pearl, Jam=Lotus, Juliett=Sunset, Rook=Corrode, Range=Practice Range, HURM_Alley=District, HURM_Yard=Piazza, HURM_Bowl=Kasbah, HURM_Helix=Drift, HURM_HighTide=Glitch |
| match_info | roster_N | match_info | JSON string: `{"name":"Sh4rgaas #EUNE","player_id":"2fb49e77-85c6-522c-a240-27c78a2f9a8f","character":"Pandemic","rank":0,"locked":false,"local":true,"teammate":true}` |
| match_info | scoreboard_N | match_info | JSON string: `{"name":"MrTest #1111","character":"Sarge","teammate":true,"team":1,"alive":true,"player_id":"61b608c1-...","shield":0,"weapon":"TX_Hud_Pistol_Classic","spike":true,"ult_points":0,"ult_max":8,"kills":11,"deaths":5,"assists":3,"money":1800,"is_local":true}` |
| match_info | round_report | match_info | JSON string with `damage, hit, headshot, bodyshots, legshots, final_headshot, damage_received, hits_received, ability_damage` (e.g. `"damage":326.3,"hit":9,"headshot":1,...`) |
| match_info | observing | match_info | `"MrTester #5555"` (who you spectate) |
| match_info | ui_team_order_allies / ui_team_order_enemies | match_info | JSON-ish string `{1:"Fade",2:"Phoenix",3:"Neon",4:"Wraith",5:"Stealth"}` |
| match_info | escalation_stage | match_info | JSON string `{"attacker":1,"defender":2}` |
| match_info | kill_feed | match_info | also mirrored as an info key (type 1) |

### Game events (type 0 keys, delivered via onNewEvents)
| Feature | Event | Payload (classic envelope, verbatim) |
|---|---|---|
| kill | kill | `{"events":[{"name":"kill","data":6}]}` (data = new total) |
| kill | assist | `{"events":[{"name":"assist","data":1}]}` |
| kill | headshot | `{"events":[{"name":"headshot","data":"1"}]}` |
| death | death | `{"events":[{"name":"death","data":14}]}` |
| match_info | match_start | `{"events":[{"name":"match_start","data":""}]}` |
| match_info | match_end | data empty; **"match_end does not work on custom matches or training"** |
| match_info | kill_feed | `{"events":[{"name":"kill_feed","data":"{\"attacker\":\"YTDestruct28\",\"victim\":\"Ghostblade\",\"is_attacker_teammate\":true,\"is_victim_teammate\":false,\"weapon\":\"TX_Hud_Volcano\",\"ult\":\"\",\"assist1\":\"TX_Killfeed_Sage1\",\"assist2\":\"\",\"assist3\":\"\",\"assist4\":\"\",\"headshot\":false}"}]}` â€” note `data` is a JSON-encoded STRING |
| match_info | spike_defused | `{"events":[{"name":"spike_defused","data":""}]}` |
| match_info | spike_detonated | `{"events":[{"name":"spike_detonated","data":""}]}` |
| match_info | planted_location | `{"events":[{"name":"planted_location","data":"A"}]}` (values A/B/C; fired "at the end of each round if the spike was planted") |
| match_info | shop | `"open"`/`"close"` (not supported in TDM) |
| match_info | scoreboard_screen | `{"events":[{"name":"scoreboard_screen","data":"open"}]}` |

**Critical schema facts for the highlight pipeline:**
- **There is NO `spike_planted` event** in the current published schema (only spike_defused / spike_detonated / planted_location). Detect plant via `round_phase` transitions + planted_location. (Insights V3's code contains a `spike_planted` string literal, but the published GEP key list does not â€” treat plant detection as derived.)
- **There are NO round_start/round_end events for Valorant** â€” use `round_phase` info-update transitions (shoppingâ†’combatâ†’endâ†’game_end).
- Agent codename map (full, from docs): Clay=Raze, Pandemic=Viper, Wraith=Omen, Hunter=Sova, Thorne=Sage, Phoenix=Phoenix, Wushu=Jett, Gumshoe=Cypher, Sarge=Brimstone, Breach=Breach, Vampire=Reyna, Killjoy=Killjoy, Guide=Skye, Stealth=Yoru, Rift=Astra, Grenadier=KAY/O, Deadeye=Chamber, Sprinter=Neon, BountyHunter=Fade, Mage=Harbor, AggroBot=Gekko, Cable=Deadlock, Sequoia=Iso, Smonk=Clove, Nox=Vyse, Cashew=Tejo, Terra=Waylay (suffix `_PC_C`).
- Limitations: roster unsupported in deathmatch & escalation; players with hidden names appear as agent name or "Me".

---

## 3. League of Legends (5426)

Docs: `.../supported-games/league-of-legends/`. 24 features (from docs + live 5426_prod.json, state=1/green): `gep_internal, live_client_data, matchState, match_info, death, respawn, abilities, kill, assist, gold, minions, summoner_info, teams, level, announcer, counters, gameMode, damage, heal, jungle_camps, team_frames, chat, panel_location, augments`.

**Events (type 0):** `matchStart`; `kill` â€” `{"events":[{"name":"kill","data":"{\"label\":\"kill\",\"count\":\"1\",\"totalKills\":\"1\"}"}]}` with label values kill/double_kill/triple_kill/quadra_kill/penta_kill (info side also keeps `doubleKillsâ€¦pentaKills` totals); `death` â€” `{"events":[{"name":"death","data":"{\"count\":\"1\"}"}]}`; `assist` â€” `{"events":[{"name":"assist","data":"{\"count\":\"1\"}"}]}`; `respawn` â€” `{"events":[{"name":"respawn","data":"{}"}]}`; `ability`/`abilityReady`/`usedAbility` â€” `{"events":[{"name":"usedAbility","data":"{\"type\":\"1\"}"}]}`; `announcer` â€” labels `welcome_rift, first_blood, victory, defeat, double_killâ€¦penta_kill, shutdown`; `match_clock` â€” `{"events":[{"name":"match_clock","data":"1249"}]}`; `chat`; `port` (live_client_data), plus per-type damage events (`physical/magic/true_damage_dealt_player`, `..._dealt_to_champions`, `..._taken`).

**Info updates (type 1):** summoner_info: `id, accountId, champion, division, level, queue, region, tier` (e.g. `{"info":{"summoner_info":{"champion":"Samira"}},"feature":"summoner_info"}`); matchState: `matchId` ("2603009084"), `queueId` ("420"), `matchStarted` ("true"); match_info: `game_mode` ("lol"), `match_paused`, `players_tagline`, `pseudo_match_id`; gold (category game_info): `{"gold":"717","total_gold":"1119"}` JSON string; minions: `minionKills`, `neutralMinionKills`; level; teams (URI-encoded JSON array of {team, champion, skinId, summoner} â€” requires `decodeURI()` + `JSON.parse()`); counters: `ping` (category performance); damage/heal totals; jungle_camps: `jungle_camp_N` â†’ `{"name":"Blue West","alive":false,"vision":false,"icon_status":"1"}`; team_frames: `team_frames_N` â†’ `{"ult_cd":47}`; augments (Arena).

**live_client_data** feature = GEP proxy of **Riot's Live Client Data API (localhost:2999)**: keys `active_player, all_players, events, game_data` (info) + `port` (event). This gives full scoreboard/items/objective events without polling Riot's API yourself.

---

## 4. CS2 (22730)

Docs: `.../supported-games/counter-strike-2/`. Live 22730_prod.json (state=1): features `match_info`, `live_data` (+ gep_internal).

**Events (all in feature match_info):** `match_start` / `match_end` (empty data), `round_start` / `round_end` (empty data), `kill` â€” `{"name":"kill","data":2}`, `death` â€” `{"name":"death","data":6}`, `assist` â€” `{"name":"assist","data":2}`, `kill_feed` (object with attacker/victim/weapon/headshot metadata). No separate bomb_planted/defused/exploded or mvp events in the published schema.

**Info updates:** match_info: `kills, deaths, assists` (ints), `game_mode` ("Competitive Dust II"), `match_outcome` ("win"/"lose"/"tie"), `elo_points` (`{"lose":136,"win":364}` â€” Premier rating deltas), `pseudo_match_id`, `roster_N` (nickname, SteamID, K/A/D, rank), `is_ranked`, `mm_state` ("searching"/"canceled"/"connect"/"unavailable"), `mode_name`, `round_number`, `round_phase`, `score`; live_data: `provider` (game/version/steamid/timestamp), `player` (health, armor, weapons, money, round stats â€” CS2's GSI-style player blob), `steam_id`, `game_phase` ("live"/"warmup"/"gameover"/"intermission"), `round_phase` ("live"/"freezetime"/"over"), `score` (`{"team_t":0,"team_ct":2}`), `map_name` ("de_dust2").

---

## 5. Apex Legends (21566)

Docs: `.../supported-games/apex-legends/`. Live 21566_prod.json (state=1): features `match_state, kill, death, me, team, revive, rank, location, roster, match_summary, kill_feed, damage, inventory, match_info, game_info` (+ gep_internal).

**Events:** `match_start`/`match_end`/`round_start`/`round_end` (empty data); `kill` â€” `{"events":[{"name":"kill","data":2}]}`; `knockdown` â€” data null; `assist` â€” data 5; `damage` â€” `{"events":[{"name":"damage","data":"{\"targetName\": \"TMW_JayJay\",\"damageAmount\": \"15.000000\",\"armor\": \"true\",\"headshot\": \"false\"}"}]}`; `death` / `knocked_out` (null); `healed_from_ko` / `respawn` (null); `kill_feed` â€” `{"events":[{"name":"kill_feed","data":"{\"local_player_name\":\"Shargaas\",\"attackerName\":\"shayan3200\",\"victimName\":\"i999n\",\"weaponName\":\"alternator\",\"action\":\"knockdown\"}"}]}` (action values include kill/knockdown/headshot variants).

**Info updates:** game_info.phase: `lobby, loading_screen, legend_selection, aircraft, freefly, landed, match_summary, shopping, combat`; me: `name`, `ultimate_cooldown` (JSON string), inventory keys `weapons` (`{"weapon0":"R-99","weapon1":"Melee"}`), `inUse`, `inventory_N`; match_info: `pseudo_match_id`, `game_mode` ("#PL_TRIO"), `tabs` (`{"kills":2,"assists":1,"teams":4,"players":10,"damage":440,"cash":10}`), `map_id` ("mp_rr_canyonlands_staging"), `arena_score`, `mode_name`, `map_name`; team: `teammate_N` (`{"name":"JacksAtWork","state":"knocked_out"}`), `legendSelect_N`, `team_info` (`{"team_state":"active"}`); roster: `roster_N` â€” `{"name":"HelloWork","isTeammate":true,"team_id":3,"platform_hw":2,"state":"knocked_out","is_local":"1","platform_id":"7656...","origin_id":"2351105644"}`; location `{"x":"93","y":"305","z":"49"}`; rank: `victory` ("false"); match_summary: `{"rank":"12","teams":"20","squadKills":"5"}`.

---

## 6. Fortnite (21216)

Docs: `.../supported-games/fortnite/`. Live 21216_prod.json (state=2): features `items, kill, match_info, rank, me, phase, location, team, killed, killer, revived, death, counters, match, map, game_info`.

**Events:** kill feature: `kill` â€” `{"events":[{"name":"kill","data":"1"}]}`, `knockout` â€” data = victim name, `hit` â€” `{"isHeadshot": false}`; `killed` â€” data = victim name; `killer` â€” data = who killed you; `revived` (empty); death feature: `death` (empty in BR; numeric in Ballistic 5v5), `knockedout` â€” data = name; match feature: `matchStart` / `matchEnd` â€” `{"events":[{"name":"matchStart","data":""}]}`; match_info feature: `generic` (values knocked, kill, 2kill, 3kill, mkill, won, death), `message_feed` ("RoDnik92 (110) has thanked the bus driver"), `emote_start` (null). NOTE: no `assist` event key in current prod status (docs show an assists info example).

**Info updates:** me: `name, health, shield, over_shield, accuracy, total_shots, xp`; rank: `rank` ("1"=Victory Royale detector), `total_teams`, `total_players` (live counters); match: `mode` (solo/duo/squad/Playlist_*); match_info: `pseudo_match_id, sessionID, matchID, userID, ticketID, partyID, match_stats` (`{"place":42,"elimination":6,"assist":0,"revive":0,"hits":51}`), `storm_info` (`{"storm_current":1,"storm_max":12,"storm_dmg":1}`), `roster_N` (`{"player":"Mefe76","team_id":102,"is_local":false}`), `skirmish`; phase: `phase` ("lobby"...), `in_vehicle`; location: `location` ({x,y,z}), `bus_line`; map: `map`, `creative_map`; game_info: `server, privacy, game_title, game_type, title_mnemonic, party_players, is_ranked, build_mode, player_rank`; items: `item_N, quickbar_N, selected_slot, selected_material`; counters: `ping`.

---

## 7. Rocket League (10798)

Docs: `.../supported-games/rocket-league/`. Live 10798_prod.json (state=1): features `stats, match, roster, me, match_info, death, game_info, training` (+ gep_internal).

**Events:** stats: `goal` â€” `{"events":[{"name":"goal","data":"{\"steamId\":\"0\",\"score\":118,\"goals\":\"1\",\"name\":\"Ram is troll\",\"team\":\"2\",\"local\":\"1\"}"}]}` (local player scored), `teamGoal`, `opposingTeamGoal` (same shape with scorer identity), `score`; match: `victory` â€” `{"events":[{"name":"victory","data":"{\"team_score\":5}"}]}`, `defeat` â€” `{"team_score":0}`, `matchStart`/`matchEnd` (empty), `overtime`, `surrender`; roster: `playerJoined`, `playerLeft`, `rosterChange`; match_info: `action_points` â€” data "Goal"; death: `death` (demolished, empty data); training: `training_round`, `training_round_result`, `training_shuffle_mode`.

**Info updates:** match: `gameMode, gameState` ("WaitingForPlayers"...), `gameType, matchType` ("Online"), `maxPlayers, ranked, started, ended`; me: `name, steamId, team, score, goals, team_score`; roster/stats: `player0..N` â€” **URL-encoded** JSON (`%7B%22steamId%22:...%7D` â€” decode with `decodeURIComponent` then JSON.parse: {steamId, score, goals, name, state, team_score, team, local, index}), `players_boost`, `players_rank`, `team1_score`, `team2_score`; match_info: `arena, mutator_settings, pseudo_match_id, server_info`; game_info: `car_look_inventory, trade_my_inventory`; training: `training_pack`.

**Bonus (from live status endpoint, for the roadmap):** Dota 2 (7314): events `game_state_changed, match_state_changed, match_ended, kill, assist, death, cs, hero_ability_used/skilled, hero_attributes_skilled, clock_time_changed, game_over, new_game`. Overwatch 2 (10844): events `kill_feed, match_start, match_end, round_start, round_end, respawn, revive, elimination, death, assist`. Marvel Rivals (24890): events `kill, death, assist, kill_feed, match_start, match_end, round_start, round_end`; infos incl. `player_stats, roster, selected_character, banned_characters, objective_progress, match_outcome`. R6 Siege (10826): events `kill, headshot, death, killer, knockedout, roundStart/roundEnd/roundOutcome/matchOutcome, match_start/match_end, defuser_planted/defuser_disabled` (defuser feature currently state=3/red). PUBG (10906): events `kill, headshot, fire, damage_dealt, knockedout, damageTaken, death, revived, killer, matchStart/matchEnd, jump`.

---

## 8. Payload envelope conventions â€” classic API vs ow-electron GEP

### 8a. Classic Overwolf API (`overwolf.games.events`, reference `https://dev.overwolf.com/ow-native/reference/games/events/`)
- `setRequiredFeatures(features: string[], callback: (Result: SetRequiredFeaturesResult) => void)` (v0.93+) â€” must be called before any events flow.
- `getInfo(callback: (Result: GetInfoResult) => void)` (v0.95+), result `{ "success": boolean, "error": string, "res": object }` â€” full current info dictionary.
- `onNewEvents` (v0.96+) envelope â€” **object with `events` array of `{name, data}`**:
```json
{"events":[{"name":"death","data":"{\"count\": \"2\"}"}]}
```
- `onInfoUpdates2` (v0.96+) envelope â€” **`{info: {<category>: {<key>: <value>}}, feature: "<feature>"}`**:
```json
{"info":{"game_info":{"minionKills":"3"}},"feature":"minions"}
```
- `onError`: `{"reason":"some reason"}`.
- **The `data`/value field is very often a JSON-encoded STRING**, confirmed by official examples across games (Valorant kill_feed, LoL kill `data:"{\"label\":\"kill\",...}"`, Apex damage/kill_feed, RL goal). Sometimes it is a bare number (`{"name":"kill","data":6}`), a bare string (`"open"`, `"A"`), or empty `""`/null. Rocket League roster values are additionally **URL-encoded** JSON. Parsers must handle: number | plain string | JSON string | URI-encoded JSON string | empty.
- Note the docs' per-game tables sometimes show a flat notation `{"feature":"x","category":"y","key":"z","value":...}` for info updates â€” that is the underlying GEP tuple (what ow-electron delivers); the classic API wraps it into the `{info:{category:{key:value}},feature}` shape.

### 8b. ow-electron GEP package (authoritative: `types.d.ts` of `@overwolf/ow-electron-packages-types@1.1.4`, modules\gep.d.ts section)
The ow-electron gep package does NOT deliver the classic envelopes. Listener signatures (verbatim from the .d.ts):
```ts
interface OverwolfGameEventPackage extends NodeJS.EventEmitter {
  getFeatures(gameId: number): Promise<string[]>;
  setRequiredFeatures(gameId: number, features: string[] | undefined): Promise<void>; // pass null/undefined = subscribe to ALL features
  getSupportedGames(): Promise<{ name: string; id: number; }[]>;
  getInfo(gameId: number): Promise<any>;
  on(eventName: 'new-info-update', listener: (event: Event, gameId: number, data: gep.InfoUpdate) => void): this;
  on(eventName: 'new-game-event', listener: (event: Event, gameId: number, data: gep.GameEvent) => void): this;
  on(eventName: 'game-detected', listener: (event: GepGameLaunchEvent, gameId: number, name: string, ...args: any[]) => void): this; // 4th arg gameInfo incl. pid, isElevated
  on(eventName: 'game-exit', listener: (event: Event, gameId: number, gameName: string, pid: number, processName: string, processPath: string, commandLine: string) => void): this;
  on(eventName: 'elevated-privileges-required', listener: (event: Event, gameId: number, name: string, pid: number) => void): this;
  on(eventName: 'error', listener: (event: Event, gameId: number, error: string, ...args: any[]) => void): this;
}
interface GepGameLaunchEvent { enable: () => void; } // call e.enable() inside 'game-detected' to activate GEP for that game

declare namespace gep {
  interface GameEvent { gameId: number; feature: string; key: string; value: any; }        // 'new-game-event' payload
  interface InfoUpdate extends GameEvent { category: string; }                              // 'new-info-update' payload
}
```
So a Valorant kill arrives in ow-electron as roughly `{gameId:21640, feature:"kill", key:"kill", value:6}` and kill_feed as `{gameId:21640, feature:"match_info", key:"kill_feed", value:"{\"attacker\":...}"}` â€” `value` still carries the JSON-string payloads shown in section 2-7. (Corroborated by Insights Capture V3, which persists exactly `video_game_event(game_class_id, feature, key, value)` and `video_game_info_update(..., category, ...)` in SQLite.)

**Wiring (official sample `overwolf/ow-electron-packages-sample`, `src/browser/services/gep.service.ts`):**
```ts
import { app as electronApp } from 'electron';
import { overwolf } from '@overwolf/ow-electron';
const app = electronApp as overwolf.OverwolfApp;

app.overwolf.packages.on('ready', (e, packageName, version) => {
  if (packageName !== 'gep') return;
  const gepApi: overwolf.packages.OverwolfGameEventPackage = app.overwolf.packages.gep;
  gepApi.on('game-detected', (e, gameId, name, gameInfo) => {
    if (!trackedIds.includes(gameId)) return;   // otherwise GEP won't attach
    e.enable();                                  // REQUIRED to start GEP for this game
  });
  gepApi.on('new-info-update', (e, gameId, ...args) => { /* args[0] = InfoUpdate */ });
  gepApi.on('new-game-event',  (e, gameId, ...args) => { /* args[0] = GameEvent  */ });
  gepApi.on('error', (e, gameId, error) => { /* reset state */ });
});
// then, after enable():
await gepApi.setRequiredFeatures(gameId, null); // null = all features (sample: setRequiredFeatures(gameId, null))
```
**Enablement:** app's package.json must declare `"overwolf": { "packages": ["gep"] }` (sample also lists "overlay"). Packages are fetched at runtime by the ow-electron package manager into `%APPDATA%\ow-electron\<uid>\packages\...\<gep-version>\` (Insights V3 observed 308.0.4 with per-game provider DLLs like `gep_valorant.dll`). DEV/QA packages via CLI flag `--owepm-packages-url=https://electronapi-qa.overwolf.com/packages` (remove for PROD).

**Versions (npm registry, live):** `@overwolf/ow-electron` dist-tags: `latest: 39.6.1` (Electron 39 line), `beta: 39.8.10-beta.11`, maintenance lines `31-x-y: 31.7.12`, `28-x-y: 28.3.8`. `@overwolf/ow-electron-packages-types`: `latest 1.1.4`. Builder: `@overwolf/ow-electron-builder`. Per-game minimum GEP runtime versions are published in the status endpoint (`min_gep_version` / `min_gep_version_electron`, e.g. Valorant 307.4.6, LoL 307.4.2, Dota 2 301.0.3/301.0.4); `disabled_electron` flags games disabled specifically on ow-electron.

**Elevated games:** if the game runs as admin, `elevated-privileges-required` fires (after `game-detected`) and the app must also run elevated to receive events.

---

## 9. Latency/reliability + health endpoint

**Latency:** No global latency SLA is documented. Docs describe both event kinds as "real time". Documented per-game caveats: LoL "Teams info-update might be provided a bit late in some game modes, as late as a few seconds before the loading screen ends"; PUBG location updates every 2s in-plane / 1s on ground; Valorant planted_location only arrives at round end. GEP disclaimer: "GEP is complex and is influenced by outside factors ... events may become temporarily unavailable" (game patches; or "a request by the relevant game studio to disable this event"). The status endpoint itself lags reality by "~10 min".

**Health endpoints (`https://dev.overwolf.com/ow-native/live-game-data-gep/game-events-status-health/`, `.../verifying-events-for-your-app/`):**
- All games: `https://game-events-status.overwolf.com/gamestatus_prod.json` â†’ JSON **array**; live entry shape today: `{"game_id":7314,"name":"Dota 2","state":1,"disabled":false,"disabled_electron":false,"is_vgep":false,"vgep_prefix":null,"min_gep_version":"301.0.3","min_gep_version_electron":"301.0.4"}` (docs also show optional `"maintenance_msg"`).
- Per game: `https://game-events-status.overwolf.com/{gameId}_prod.json` (e.g. `21640_prod.json`, `5426_prod.json`). Live shape:
```json
{"game_id":5426,"name":"League of Legends","state":1,"disabled":false,"disabled_electron":false,"published":true,"is_vgep":false,
 "features":[{"name":"summoner_info","state":1,"published":true,
   "keys":[{"name":"champion","type":1,"state":1,"is_index":false,"category":"summoner_info","sample_data":"Katarina","status_comment":null,"published":true,"is_vgep":false}, ...]}]}
```
- `state` enum (documented): **0 = unsupported, 1 = green (Good to go), 2 = yellow (Partial functionality, some game events may be unavailable), 3 = red (Game events are unavailable)** â€” applied at game, feature, and key level.
- `type` on keys is undocumented but empirically **0 = game event (onNewEvents/new-game-event), 1 = info update** (verified: all `summoner_info` keys are type 1; `kill`/`ability` event keys type 0; Valorant `kill` type 0 vs `kills` type 1). Extra live fields per key: `is_index` (numbered keys like roster_N/scoreboard_N), `category`, `sample_data`, `status_comment`, `published`, `is_vgep`.
- Poll this endpoint from the companion app to gate features and surface degraded-event warnings (e.g., today Valorant is state 2 because `me.abilities` is state 3).

**Testing without a live game:** Overwolf's GEP Simulator app â€” `https://dev.overwolf.com/ow-native/guides/dev-tools/using-the-game-events-simulator-app/`; sample event apps: `https://github.com/overwolf/events-sample-apps`.

## 10. Implications for the OBS-plugin + companion-app design
- The companion (ow-electron) will receive **flat `{gameId, feature, key, value(, category)}` tuples**, not the classic `{events:[...]}` envelope â€” define the localhost-WebSocket protocol around that tuple + a monotonic timestamp, and JSON-parse `value` opportunistically (number | string | JSON-string | URI-encoded JSON-string | "").
- For Valorant highlight triggers use: `kill/assist/headshot/death` (counters), `kill_feed` (rich victim/attacker/weapon/headshot), `round_phase`, `match_start/match_end`, `spike_defused/spike_detonated/planted_location`, `score/match_score`, `scoreboard_N/roster_N`, `agent`, `game_mode`, `match_id/pseudo_match_id`. Derive Ace/Multikill/Clutch by aggregating kills within a round (Insights V3 does exactly this; its clip window default is 5s pre / 3s post event).
- `setRequiredFeatures(gameId, null)` (all features) is the simplest robust subscription; retry after game launch if it fails (features can register late).
