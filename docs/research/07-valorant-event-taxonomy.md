# Insights Capture V3 â€” Per-game highlight-event taxonomy (mined from workspace)

All paths relative to `G:\personal\insights_capture\`. Key bundles: `extracted\app_asar\out\renderer\assets\loaders-BeVBhtxu.js` (settings engine, 1.8 MB), `...\gameEventStatus-CEdsQP3k.js`, `...\index-C2RRUDad.js` (timeline icons), `...\VideoReplay-Dj7NYuGr.js` (timeline adapters), `notes\index.jsc.strings.txt` (main-process strings). App codename leaked in jotai debug labels: `D:\a\turbo-octo-broccoli\turbo-octo-broccoli\src\renderer\src\...` (index-C2RRUDad.js:~78678).

## 1. Per-game highlight event CHOICES (user-selectable auto-highlight events)

Source of truth: `loaders-BeVBhtxu.js` lines **40204â€“40664**. Selection function (line 40662):
```js
function getHighlightEvents(gameClassId) {
  return timelineHighlightEvents[gameClassId] ?? nonTimelineSupportedEvents[gameClassId] ?? DEFAULT_HIGHLIGHT_EVENT;
}
```
`DEFAULT_HIGHLIGHT_EVENT = [{value:"bookmark", label:"Bookmark"}]` (line 40204). Every game's list starts with `bookmark`. Labels below are the hardcoded English fallbacks; at render time labels come from i18n `t(\`gameEvents:${e.value}\`)` (index-C2RRUDad.js:75817).

**timelineHighlightEvents** (line 40622 â€” games with full timeline UI):

| Game (id) | machine_key â†’ label | Evidence line |
|---|---|---|
| CS2 (22730) | bookmarkâ†’Bookmark, killâ†’Kill, deathâ†’Death, assistâ†’Assist | 40294 |
| League of Legends (5426) | bookmarkâ†’Bookmark, killâ†’Kill, deathâ†’Death, assistâ†’Assist, AtakhanKillâ†’Atakhan, BaronKillâ†’Baron, DragonKillâ†’Dragon, HeraldKillâ†’Herald, HordeKillâ†’Voidgrubs, InhibKilledâ†’Inhibitor, TurretKilledâ†’Turret | 40208 |
| VALORANT (21640) | bookmarkâ†’Bookmark, killâ†’Kill, deathâ†’Death, assistâ†’Assist | 40242 |

**nonTimelineSupportedEvents** (map at 40627â€“40661):

| Game (id) | machine_keys (label) | Line |
|---|---|---|
| Apex Legends (21566) | bookmark, kill (Kill), death (Death), assist (Assist), knockdown (Knockdown), knocked_out (Knocked Out) | 40255 |
| Arc Raiders (27168) | bookmark, death, extraction (Extraction) | 40274 |
| Black Myth Wukong (24504) | bookmark, kill, death | 40284 |
| Dark and Darker (22584) | bookmark, death | 40307 |
| Deadlock (24482) | bookmark, kill, death, assist | 40314 |
| Diablo IV (22700) | bookmark, death | 40327 |
| Dota 2 (7314) | bookmark, kill, death, assist | 40334 |
| Elden Ring Nightreign (25918) | bookmark, boss_fight (Boss Fight), death | 40347 |
| Escape From Tarkov (21634) | bookmark, death | 40357 |
| Fortnite (21216) | bookmark, kill, death, knockout (Knock Out), knockedout (Knocked Out) | 40364 |
| Genshin Impact (21656) | bookmark, dungeon_start (Dungeon Start) | 40380 |
| HELLDIVERS 2 (24000) | bookmark, death | 40387 |
| Honkai Star Rail (22804) | bookmark, battle_start (Battle Start) | 40394 |
| Lethal Company (23522) | bookmark, death, respawn (Respawn) | 40401 |
| Marvel Rivals (24890) | bookmark, kill, death, assist | 40411 |
| Once Human (23930) | bookmark, death, knockout (Knock Out) | 40424 |
| Overwatch 2 (10844) | bookmark, kill, death, revive (Revive) | 40434 |
| Palworld (23944) | bookmark, death, knock_out (Knock Out), revive (Revive) | 40447 |
| Path of Exile (7212) | bookmark, boss_kill (Boss Kill), death | 40460 |
| Path of Exile 2 (24886) | bookmark, boss_kill, death | 40470 |
| PEAK (26092) | bookmark, death, teammate_death (Teammate Death), checkpoint (Checkpoint) | 40480 |
| PUBG (10906) | bookmark, kill, death, knockout, knockedout | 40493 |
| Rainbow Six Siege (10826) | bookmark, kill, death, knockedout (Knocked Out), roundstart (Round Start) | 40509 |
| R.E.P.O (25448) | bookmark, round_start (Round Start), death | 40525 |
| Rocket League (10798) | bookmark, teamGoal (Team Goal), opposingTeamGoal (Opponent Team Goal), death | 40535 |
| Sons of the Forest (22638) | bookmark, death | 40548 |
| Splitgate 2 (25884) | bookmark, kill, death, round_start | 40555 |
| Street Fighter 6 (22894) | bookmark, round_start | 40568 |
| Teamfight Tactics (21570) | bookmark, battle_start | 40575 |
| The Finals (23478) | bookmark, elimination (Elimination), death | 40582 |
| Warhammer 40K Space Marine 2 (24548) | bookmark, death, knockout (Knockout) | 40592 |
| Wuthering Waves (24300) | bookmark, death | 40602 |
| 2XKO / XKO (26840) | bookmark, round_start, assist, character_switch (Character Switch) | 40609 |

Games with highlight support but **only bookmark** choice (fall through to DEFAULT): HaloInfinite (21854), HadesII (24218), and everything else in `recordingModeGameSupport` (Baldur's Gate III 22088, Content Warning 24110, FFXIV 6350, Hearthstone 9898, Lost Ark 21864, MTG Arena 21308, Manor Lords 24176, Minecraft 8032, New World 21816, Roblox 4688, Starfield 23222, The First Descendant 24360, Warframe 8954, WoW 765, Arknights Endfield 27724) â€” map at loaders lines 40727â€“40944 with per-game `{autoRecord, highlights}` flags (autoRecord:false for FFXIV, MTGA, NewWorld, PathofExile, Warframe, WoW).

### Timeline event whitelist per game (what gets stored/shown on the timeline â€” superset of choices)
`gameEventStatus-CEdsQP3k.js` lines **12578â€“12616**: `GamesWithGameEvents` (36 games) + `eventsToKeepByGameId`:
```js
XKO: ["assist","round_start","character_switch"]
ApexLegends: ["kill","death","assist","knockdown","knocked_out"]
ArcRaiders: ["extraction","death"]
BlackMythWukong: ["kill","death"]
CS2: ["kill","kill_feed","death","assist","round_start"]
DarkandDarker: ["death"]
Deadlock: ["kill","death","assist","match_start"]
DiabloIV: ["death"]
Dota2: ["kill","death","assist"]
ELDENRINGNIGHTREIGN: ["boss_fight","death"]
EscapeFromTarkov: ["death"]
Fortnite: ["kill","knockout","death","knockedout"]
GenshinImpact: ["dungeon_start"]
HadesII: ["death"]
HaloInfinite: ["kill","death","assist"]
HELLDIVERS2: ["death"]
HonkaiStarRail: ["battle_start"]
LeagueofLegends: ["kill","death","assist","victory","match_clock","defeat","events", ...LEAGUE_TIMELINE_EVENTS]
LethalCompany: ["death","respawn"]
MarvelRivals: ["kill","death","assist","match_start"]
OnceHuman: ["knockout","death"]
Overwatch: ["kill_feed","elimination","assist","death","round_start"]
Palworld: ["knockout","death","revive","respawn"]
PEAK: ["death","teammate_death","checkpoint"]
PathofExile / PathofExile2: ["boss_kill","death"]
PUBG: ["kill","death","knockout","knockedout"]
Rainbow6Siege: ["kill","death","knockedout","roundStart"]
REPO: ["death"]
RocketLeague: ["teamGoal","opposingTeamGoal","death","action_points"]
SonsoftheForest: ["death"]
TeamfightTactics: ["battle_start"]
TheFinals: ["elimination","death"]
VALORANT: ["kill","death","assist","ability_used","spike_planted","kill_feed"]
Warhammer40000SpaceMarine2: ["knockout","death"]
WutheringWaves: ["death","respawn"]
```
`LEAGUE_EXTRA_GAME_EVENTS = ["AtakhanKill","BaronKill","DragonKill","HeraldKill","HordeKill","InhibKilled","TurretKilled"]`; `LEAGUE_TIMELINE_EVENTS = [...that, "ChampionKill","Multikill","Ace"]` (lines 12573â€“12574).

### Sub-taxonomies inside event values
- **LoL** `events` feature carries `{EventName, KillStreak}`: EventName âˆˆ ChampionKill/Multikill/Ace/Atakhan.../TurretKilled; `Multikill` KillStreak 2â†’DoubleKill, 3â†’TripleKill, 4â†’QuadraKill, 5â†’PentaKill icons (VideoReplay-Dj7NYuGr.js:5572â€“5625; adapter 7090â€“7123; bookmark filter drops Multikill/Ace from bookmarkable events, gameEventStatus:12724â€“12745).
- **Rocket League** `action_points` value.EventName âˆˆ "Assist","Center Ball","Demolish","Epic Save","First Touch","Goal","Save","Shot On Goal"; also timeline keys `goal`,`teamGoal`,`opposingTeamGoal`,`matchStart` (index-C2RRUDad.js:78419â€“78445 region, getRocketLeagueEventIcon).
- **Valorant**: timeline renders keys kill/death/assist/ability_used/spike_planted + `round_number` info-updates (adapter VideoReplay:9761â€“9784); `round_report` info updates carry `{hit, headshot, final_headshot, damage}` (VideoReplay:7872â€“7897); kill_feed rendered with `IconHeadshot` when headshot flag set (VideoReplay:9525â€“9581). Dictionaries: `VALORANT_AGENT_NAME_DICT` (internal codenameâ†’agent, e.g. Clayâ†’Raze, Wushuâ†’Jett, Irisâ†’Miks), `VALORANT_ULT_DICT` (TX_* texture â†’ ability name), `VALORANT_WEAPON_DICT` (TX_Hud_* â†’ weapon) in gameEventStatus-CEdsQP3k.js:12630â€“12723.
- **Per-game icon switches** (another event-key inventory): `index-C2RRUDad.js` `get<Game>EventIcon` functions ~76822 (2XKO) and 77591â€“78614, `GAME_EVENT_ICON_MAP` at 78615â€“78655 (38 games + `0:` bookmark fallback).

## 2. Default autoHighlightEvents (verbatim)

`loaders-BeVBhtxu.js:40945â€“41051` â€” `perGameHighlightEventsDefaults(gameClassId)` returns `events.map(e => [e, {enabled:1, seekBeforeAmount:5, amountToWatchAfterBookmark:3}])`:

| Game | default events |
|---|---|
| ApexLegends | ["kill","knockdown"] |
| ArcRaiders | ["death"] |
| BlackMythWukong / CS2 / Deadlock / Dota2 / LoL(+PBE) / MarvelRivals / Rainbow6Siege / Splitgate2 / VALORANT | ["kill"] |
| Fortnite / PUBG | ["kill","knockout"] |
| OnceHuman / Warhammer40000SpaceMarine2 | ["knockout"] |
| Overwatch | ["elimination"] |
| Palworld | ["knock_out"] |
| PathofExile / PathofExile2 | ["boss_kill"] |
| REPO | ["death"] |
| RocketLeague | ["teamGoal"] |
| TeamfightTactics | ["battle_start"] |
| TheFinals | ["elimination"] |
| DarkandDarker, DiabloIV, ELDENRINGNIGHTREIGN, EscapeFromTarkov, HELLDIVERS2, LethalCompany, PEAK, SonsoftheForest, WutheringWaves (and any game without a case: Genshin, HonkaiStarRail, SF6, XKO, Halo, Hadesâ€¦) | [] |

Setting definition `autoHighlightEventsSetting` (loaders 41721â€“41796): `path: settingPath.recording.highlights.autoHighlightEvents`, type multi-select "string-with-options", per-event options `enabled` (0/1), `seekBeforeAmount` (number input, **min 0 max 20 step 1**), `amountToWatchAfterBookmark` (min 0 max 20 step 1); global `defaultValueWithOptions: ["bookmark", {enabled: 0, seekBeforeAmount: 5, amountToWatchAfterBookmark: 3}]`.

Stored DB form (migration strings `notes\20260529190852_update_autoHighlightEvents_to_correct_form-DchgDFz3.jsc.strings.txt`, lines 24â€“32): setting `recording.highlights.autoHighlightEvents` = JSON array of `[eventName, {"enabled":1,"seekBeforeAmount":5,"amountToWatchAfterBookmark":3}]`. Related paths from `notes\20260515185128_match_old_highlights_behaviour-C5PgE6tQ.jsc.strings.txt`: `recording.mode` ('auto'|'highlight'|'full'|'manual'|'clips'), `recording.highlights.enabled`, `recording.highlights.deleteOriginal`, `recording.highlights.timings.seekBeforeAmount|amountToWatchAfterBookmark`; setting table keyed by (path, game_class_id, queue_id). English labels (settings-Bwn9KyA8.js): autoHighlightEvents="Auto highlight events", enabled="Enable auto highlight", seekBeforeAmount="Seek before game event", amountToWatchAfterBookmark="Watch after game event".

## 3. Game-ID map

`kGameIds` is the full Overwolf registry embedded in the renderer â€” thousands of entries, `loaders-BeVBhtxu.js:36831â€“40723`; `kLaunchersIds = {LOLLauncher: 10902}` (36828); `kGameIdsExtension = {ArknightsEndfield: 27724}` (40724). IDs relevant to events/highlights:

| Name | id | | Name | id |
|---|---|---|---|---|
| WoW | 765 | | LethalCompany | 23522 |
| Unreal3 | 3248 | | TheFinals | 23478 |
| Roblox | 4688 | | Starfield | 23222 |
| LeagueofLegends | 5426 | | OnceHuman | 23930 |
| FinalFantasyXIVOnline | 6350 | | Palworld | 23944 |
| PathofExile | 7212 | | ContentWarning | 24110 |
| Dota2 | 7314 | | ManorLords | 24176 |
| CSGO | 7764 | | HadesII | 24218 |
| Minecraft | 8032 | | WutheringWaves | 24300 |
| Warframe | 8954 | | TheFirstDescendant | 24360 |
| HearthstoneHeroesofWarcraft | 9898 | | Deadlock | 24482 |
| RocketLeague | 10798 | | BlackMythWukong | 24504 |
| Rainbow6Siege | 10826 | | Warhammer40000SpaceMarine2 | 24548 |
| Overwatch | 10844 | | PathofExile2 | 24886 |
| PUBG | 10906 | | MarvelRivals | 24890 |
| Fortnite | 21216 | | REPO | 25448 |
| MagictheGatheringArena | 21308 | | Splitgate2 | 25884 |
| OverwatchPublicTestRegion | 21438 | | ELDENRINGNIGHTREIGN | 25918 |
| ApexLegends | 21566 | | PEAK | 26092 |
| TeamfightTactics | 21570 | | XKO (2XKO) | 26840 |
| EscapeFromTarkov | 21634 | | ArcRaiders | 27168 |
| VALORANT | 21640 | | ArknightsEndfield | 27724 |
| GenshinImpact | 21656 | | LeagueofLegendsPBE | 22848 |
| LostArk | 21864 | | StreetFighter6 | 22894 |
| HaloInfinite | 21854 | | VALORANTPBE | 22904 |
| NewWorld | 21816 | | HonkaiStarRail | 22804 |
| BaldursGateIII | 22088 | | DiabloIV | 22700 |
| DarkandDarker | 22584 | | CS2 | 22730 |
| SonsoftheForest | 22638 | | HELLDIVERS2 | **24000** (`24e3` literal, loaders:38125) |

Installer-level detection DB `extracted\nsis\$PLUGINSDIR\app\game_detection_database.json` â€” 40 `{id,name,gameDetectionHints[]}` entries (registry/path hints): 765 WoW, 21630 WoW Classic, 8032 Minecraft, 5426 LoL, 21216 Fortnite BR, 21566 Apex, 21640 VALORANT, 10826 R6 Siege, 10798 Rocket League, 10540 Sims 4, 23944 Palworld, 21656 Genshin (Ã—2 dup), 8954 Warframe, 7314 Dota 2, 22730 CS2, 23476 ARK SA, 10778 ARK SE, 22088 BG3, 23522 Lethal Company, 21864 Lost Ark, 4871 MTG Battlegrounds, 21812 Destiny 2, 21816 New World, 9898 Hearthstone, 21650 Fall Guys, 21866 Elden Ring, 23222 Starfield, 22262 Marvel Snap, 22700 Diablo IV, 21634 Tarkov, 24890 Marvel Rivals, 4688 Roblox, 10844 Overwatch, 26462 Battlefield 6, 27168 Arc Raiders, 24884 Delta Force, 24886 PoE2, 25448 R.E.P.O, 26926 Megabonk.

Per-game **game-mode recording filters** (queue/mode gating, settings-Bwn9KyA8.js, `recording.gameModeRecording.gameModes.choices`, used by `gameModeRecordingSetting` loaders:41680â€“41714 for Apex/LoL/OW2/R6/Valorant): LoL 5426 queue ids 400 Draft, 420 Ranked Solo/Duo, 440 Ranked Flex, 450 ARAM, 480 Swiftplay, 900 ARURF, 1700 Arena, custom, other; Valorant 21640: competitive, custom, deathmatch, escalation, quick_bomb (Spike Rush), range (Practice Range), swift (Swiftplay), team_deathmatch, unrated, other; Apex 21566: `#PL_Ranked_Leagues` Ranked, `#PL_DUO`/`#PL_TRIO`, `#GAMEMODE_ARENAS(_RANKED)`, `#GAME_MODE_GUNGAME` Gun Run, `#CONTROL_NAME`, `#TDM_NAME`, `#PL_FIRINGRANGE`, `#PL_TRAINING`; R6 10826: RANKED/UNRANKED/QUICK MATCH/DEATHMATCH/DUAL FRONT/CUSTOM/SHOOTING RANGE/FIELD TRAINING/LANDMARK DRILL/CLEAR HOUSE; OW2 10844: RANKED/UNRANKED/ARCADE/CUSTOM_GAME/DEATHMATCH/HERO_MASTERY(_SOLO)/PRACTICE/SKIRMISH/TUTORIAL/VS_AI/UNKNOWN.

## 4. setRequiredFeatures / GEP features

Main process (`notes\index.jsc.strings.txt`):
- gepService flow strings (5338â€“5369): `match-start`/`match_started`, `match-end`/`match_ended`, `gep_error`, `game-detected`, `game not enabled`, `setting features`, **`setRequiredFeatures`** (5352), `GEP:game-enabled-set. game not enabled or not active`, `setting features because game-enabled-set`, `new-game-event`, `new-info-update`, `setStickyInfoUpdate`, `game-exit`, `clearStickyInfoUpdates`; API surface: `gepApi, getFeatures, getSupportedGames, onGameDetected, onGameExit, onNewGameEvent, onNewInfoUpdate, onElevatedPrivilegesRequired` (5411â€“5441).
- **The only contiguous feature array in the dump** (12073â€“12085): `gep_internal, game_info, match_info, matchState, match_state, roster, live_client_data, minions, counters, announcer, summoner_info, lobby_info, end_game`. This reads as the LoL-oriented required-features list (live_client_data/summoner_info/announcer/minions/counters are LoL Live Client Data features). Caveat: .jsc constant pools are per-function, but no other feature-name runs exist in the dump â€” so other games are likely subscribed with no explicit list (= all features) or the list is assembled from these same strings. Event *keys* per game are definitively the ones in Â§1's `eventsToKeepByGameId`.
- GEP health endpoints (2047â€“2056): `overwolfAllGameStatusUrl = https://game-events-status.overwolf.com/gamestatus_prod.json`, per-game `overwolfDetailedGameStatusUrl = https://game-events-status.overwolf.com/{gameClassId}_prod.json`; renderer also calls `api.overwolf.getGepSupportedGames()` (loaders:41872) and game icons `https://static.overwolf.com/GameIcons/${gameClassId}.png` (gameEventStatus:12189).
- GEP state helpers: `game_state`, `game_state_changed`, `playing`, `isMatchStartEvent` (`matchStart|match_start`), `isMatchEndEvent` (`match_end|matchEnd`) (5400â€“5410).

## 5. OBS / ow-obs-websocket

- Nothing about `ow-obs-websocket` ports/protocol in the app bundles â€” the control channel lives in the native **recorder** package (`%APPDATA%\ow-electron\<uid>\packages\jjcifbonâ€¦\0.32.44` per `notes\TECHNICAL_TEARDOWN.md:169,375`: full OBS + `ow-electron-obs.exe` + `ow-obs-websocket.dll`).
- The app *does* pass OBS encoder ids through settings (notes\index.jsc.strings.txt:1219â€“1262): `jim_nvenc`, `jim_hevc_nvenc`, `jim_av1_nvenc`, `obs_nvenc_h264_tex`, `obs_nvenc_hevc_tex`, `obs_nvenc_av1_tex`, `obs_qsv11_v2`, `obs_qsv11_hevc`, `obs_qsv11_av1`, `h264_texture_amf`, `h265_texture_amf`, `av1_texture_amf`, `obs_x264`, `ffmpeg_svt_av1`, plus OBS-shaped props `rate_control, multipass, tune, target_usage, colorFormat NV12, colorRange Partial`. So Insights drives Overwolf's OBS through the ow-electron recorder API using **libobs encoder identifiers verbatim** â€” an OBS bridge can mirror these names 1:1.
- Recording modes (loaders:41862â€“41919): `auto | highlight | full | manual | clips`; instant replay durations [15,30,60,120,300,600]s (41823); `recorderStats`, `recorder_package_version` in main strings (4770, 4813).

## 6. i18n display names

- Event-name locale bundles: `extracted\app_asar\out\renderer\assets\gameEvents-*.js` â€” 11 locales, flat `machine_key â†’ label`. **English = gameEvents-BmQQ7Gvy.js** (47 keys): Ace, AtakhanKillâ†’Atakhan, BaronKillâ†’Baron, ChampionKillâ†’Champion Kill, DragonKillâ†’Dragon, HeraldKillâ†’Herald, HordeKillâ†’Voidgrub, InhibKilledâ†’Inhibitor, Multikillâ†’Multi Kill, TurretKilledâ†’Turret, ability_usedâ†’Ability Used, action_pointsâ†’Action Points, assistâ†’Assist, battle_startâ†’Battle Start, bookmarkâ†’Bookmark, boss_fightâ†’Boss Fight, boss_killâ†’Boss Kill, character_switchâ†’Character Switch, checkpointâ†’Checkpoint, deathâ†’Death, defeatâ†’Defeat, dungeon_startâ†’Dungeon Start, eliminationâ†’Elimination, eventsâ†’Events, extractionâ†’Extraction, goalâ†’Goal, killâ†’Kill, kill_feedâ†’Kill Feed, knock_outâ†’Knock Out, knockdownâ†’Knockdown, knocked_outâ†’Knocked Out, knockedoutâ†’Knocked Out, knockoutâ†’Knock Out, matchStart/match_start/matchstartâ†’Match Start, match_clockâ†’Match Clock, opposingTeamGoalâ†’Opposing Team Goal, respawnâ†’Respawn, reviveâ†’Revive, roundStartâ†’"Rount Start" (typo in EN), round_start/roundstartâ†’Round Start, spike_plantedâ†’Spike Planted, teamGoalâ†’Team Goal, teammate_deathâ†’Teammate Death, victoryâ†’Victory. Other locales: 04xnDcNU zh, B1sjf1KV ko, B2lN5k3J tr, BLC7fetN es, BLnZvy_N de, BYhulFwA ru, C0q8zoPa pt, Hc0SY0P3 vi, qTxTsERX ja, yJmMXcqq fr/pl.
- Settings labels per locale: `settings-*.js` (EN = settings-Bwn9KyA8.js).
- `extracted\nsis\$PLUGINSDIR\app\_locales\en\messages.json` = **installer-only** strings (89 keys, btn_install etc.) â€” no event names.

## 7. Surprises / corrections

1. **No clutch/1vX/MVP/ace derivation exists in V3.** Exhaustive search of all renderer bundles finds zero `clutch`, `1v1..1v5`, `MVP` event tokens (only Valorant voice-keybind names "Team/Party Clutch Mute", index.jsc.strings.txt:12233â€“12238, and SVG path false-positives). "Ace"/"Multikill" exist **only for LoL**, delivered by Riot's Live Client Data `events` feature (EventName), not computed by Insights. This corrects `notes\REVERSE_ENGINEERING_FINDINGS.md` Â§4/TL;DR item 4 which claimed client-side derived Ace/Multikill/Clutch/MVP for Valorant.
2. **`headshot` is not an event key** â€” it's a boolean flag inside `kill_feed` values and `round_report` counters (`headshot`, `final_headshot`), rendered as an icon modifier.
3. **Overwatch mismatch:** default auto-highlight is `["elimination"]` but its *choices* array offers only bookmark/kill/death/revive â€” `elimination` default can't be unticked from the standard choice list (elimination/kill treated as same icon: index-C2RRUDad.js getOverwatchEventIcon `case "elimination": case "kill":`).
4. Key-name inconsistency across games is real and must be preserved by a bridge: `knockdown` (Apex) vs `knockout` (Fortnite/PUBG/OnceHuman/WH40K) vs `knock_out` (Palworld) vs `knockedout` (Fortnite/PUBG/R6) vs `knocked_out` (Apex); `roundstart` (R6) vs `roundStart` (R6 DB filter) vs `round_start` (CS2/OW2/REPO/SF6/Splitgate2/XKO).
5. LoL bookmark/highlight filtering deliberately **drops** `Multikill`/`Ace` from bookmarkable events (`filterEventsForBookmarks`, gameEventStatus:12724â€“12745) and Valorant drops `round_number`; SQL in main does the same (`NOT IN ('ChampionKill','Multikill','Ace','TurretKilled')`, index.jsc.strings.txt:1546).
6. Timeline UIs exist for only 4 games (CS2, LoL, OW2+PTR 21438, Valorant â€” `registerTimeline` in VideoReplay-Dj7NYuGr.js); every other game gets icon-only event rows.
7. Streamer-mode self-name detection list `POSSIBLE_STREAMER_MODE_NAME = ["Me","è‡ªåˆ†","Ø£Ù†Ø§","Ich","Yo","Moi","Aku","Giocatore","ë‚˜","Ja","Eu","ÐœÐ¾Ð¹ Ð°Ð³ÐµÐ½Ñ‚","à¸‰à¸±à¸™","Ben","TÃ´i","æˆ‘"]` (gameEventStatus:12623) â€” needed to attribute kill-feed lines to the local player when Valorant streamer mode hides names.
8. HELLDIVERS2 id is `24e3` (24000) â€” grep for plain integers misses it.
9. Migration `20251015235526_enabled_games` shows per-game enable/UX state: `games.enabledGames`, `visiblyEnabled`, `choseRecordingModeInNotification`, `dismissedGames`; `20251217190413_game_support` seeds per-game `{autoRecord, highlights}` and prunes `recording.mode` settings for unsupported games.
10. DB event model (for the bridge): events land as `video_game_event(game_class_id, feature, key, value, timestamp)` and info updates as `video_game_info_update(..., category)`; LoL `events` values are re-timestamped via `EventTime` deltas (index.jsc.strings.txt:1470â€“1497).

