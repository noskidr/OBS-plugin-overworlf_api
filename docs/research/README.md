# Research reports

Primary‑source research gathered before and during implementation (each verified
against official docs / source as of 2026‑07‑03). These are the evidence base for
the design decisions in `CLAUDE.md` and the code — when in doubt about *why*
something works the way it does, the answer is cited here.

> Note: some reports contain minor mojibake (em‑dashes rendered as `â€"`) from
> the extraction round‑trip; the technical content is accurate.

| # | File | Covers |
|---|---|---|
| 01 | [ow-electron GEP API](01-ow-electron-gep-api.md) | `@overwolf/ow-electron` packages/versions, the exact GEP JS API (`app.overwolf.packages`, `game-detected`→`enable()`, `setRequiredFeatures`), dev vs prod gating, licensing, overlay/recorder packages, classic‑vs‑ow‑electron comparison + recommendation. |
| 02 | [GEP games & event schemas](02-gep-games-event-schemas.md) | Supported games + numeric ids, and the **full Valorant event/info schema** (kill/death/kill_feed/spike/round_phase/scoreboard/roster…), plus LoL/CS2/Apex/Fortnite/Rocket League; payload envelope conventions; the `game-events-status` health endpoint. |
| 03 | [OBS frontend API](03-obs-frontend-api.md) | Verified signatures for `obs_frontend_recording_add_chapter`, replay buffer, recording/streaming state + elapsed time, frontend event enum, docks, hotkeys, captions; OBS version gates (chapters = 30.2+, dock = 30.0+). |
| 04 | [OBS plugin engineering](04-obs-plugin-engineering.md) | obs‑plugintemplate current state (presets, buildspec pins, ENABLE_QT/FRONTEND, CI), WebSocket‑server options inside a plugin, real dock plugins to copy, overlay‑source rendering patterns, threading rules (`obs_queue_task`), config persistence. |
| 05 | [Market research](05-market-research.md) | What streamers/editors actually want from game‑event‑driven OBS automation (2024‑2026 demand), the competitive gap (no tool bridges Overwolf events into OBS), and novel feature ideas. |
| 06 | [Twitch / YouTube platforms](06-twitch-youtube-platforms.md) | Twitch Helix stream markers (scope, constraints), Device Code Grant OAuth for a native app, anonymous IRC `!clip`, EventSub channel points; YouTube chapters (no marker API); timestamp math; Kick. |
| 07 | [Valorant event taxonomy](07-valorant-event-taxonomy.md) | The per‑game highlight‑event taxonomy **mined from the insights.gg V3 app** — machine keys → labels, default `autoHighlightEvents`, game‑id map, `setRequiredFeatures` arrays. Defines the product's event vocabulary. |
| 08 | [Gap‑analysis critic](08-gap-analysis-critic.md) | Adversarial review of reports 01‑07 before implementation: critical unknowns, contradictions, unflagged risks (ToS, marker limits, forgery via localhost WS), and the 3 things to verify *during* implementation. |

How these were produced: a dynamic multi‑agent research workflow (7 parallel
researchers + a completeness critic), each instructed to prefer primary sources
and quote exact signatures/URLs. The same harness later ran the adversarial code
review (`docs/REVIEW-FINDINGS.md`).
