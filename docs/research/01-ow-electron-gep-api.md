# Overwolf ow-electron ("OWElectron") â€” Complete API Research Report

All findings verified against primary sources (npm registry, official docs at dev.overwolf.com, official GitHub repos) as of **2026-07-03**.

---

## 1. npm packages and versions

| Package | Latest stable | Beta / other dist-tags | License (npm) | Purpose |
|---|---|---|---|---|
| `@overwolf/ow-electron` | **39.6.1** (`latest`) | `beta: 39.8.10-beta.11`, `31-x-y: 31.7.12`, `28-x-y: 28.3.8`, `12-x-y: 12.2.3-2` | MIT (wrapper; runtime services are closed-source) | Drop-in Electron fork with Overwolf services |
| `@overwolf/ow-electron-builder` | **26.8.5** (`latest`) | `beta: 26.9.0-beta.4`, `next: 25.0.5`, `alpha: 26.0.11-alpha.0` | MIT | Fork of `electron-builder` that packages ow-electron apps |
| `@overwolf/ow-electron-packages-types` | **1.1.4** (`latest`) | `beta: 1.1.5-beta.2` | MIT | TS typings for the runtime packages (gep/overlay/recorder/utility/crn) + game-id enums |
| `@overwolf/electron-is-overwolf` | **0.0.2** | â€” | MIT | `electron-is-dev`-style check: "Checks if Electron is Overwolf build (ow-electron)" |

Notes:
- The `@overwolf/ow-electron` major version tracks the underlying Electron major (dist-tags per Electron line; changelog entries literally say e.g. "Updated the underlying Electron version to 37.10.3", 34.5.5, 34.4.1, etc.). Latest stable = Electron 39 line.
- Its npm metadata is inherited from electron's wrapper (`repository: git+https://github.com/electron/electron.git`, description "Build cross platform desktop appsâ€¦"); the real vendor repos are `github.com/overwolf/*`.
- FAQ (official): "Overwolf Electron is a fork of the Electron.js project, complete with built-in integration with several of Overwolf's services." â€¦ "Why is it closed source? Overwolf Electron utilizes several of our in-house, proprietary services and therefore its not possible to make them open source."
- Official docs list exactly three ecosystem packages (Phase 2 page + technical overview): `@overwolf/ow-electron` ("a package based on the electron package, adding several new features to it"), `@overwolf/ow-electron-builder` ("based on the electron-builder package, which supports building ow-electron apps"), `@overwolf/electron-is-overwolf`.

### How it replaces stock electron
From the official sample repo `overwolf/ow-electron-packages-sample` (package.json, verbatim excerpts):

```json
"devDependencies": {
  "@overwolf/ow-electron": "latest",
  "@overwolf/ow-electron-builder": "latest",
  "@overwolf/ow-electron-packages-types": "^0.0.5",
  "electron": "21.1.0",
  ...
},
"scripts": {
  "start": "ow-electron .",
  "build:start": "yarn run build && ow-electron . ",
  "build:ow-electron": "ow-electron-builder --c.extraMetadata.name=GameEventsTester --publish=never"
}
```
i.e. you keep writing a normal Electron app, but launch with the **`ow-electron` CLI binary** instead of `electron`, and package with **`ow-electron-builder`** instead of `electron-builder`. (Stock `electron` may coexist as devDependency for typings/tooling; FAQ: "As a fork of Electron.js we ensure upstream compatibility for specific versions of Electron.js.") `@overwolf/electron-is-overwolf` lets shared code detect which runtime it's on.

- App identity: the app UID is derived from `productName` (falls back to `name`) + `author.name` in package.json. Constraint (docs, first-app page): "You can't use an app name that contains the word bot". At runtime:
```javascript
import app from 'electron';
app.whenReady().then(() => {
  const appID = process.env.OVERWOLF_APP_UID;  // exists only after app init
});
```
- No manifest.json (FAQ): "No, Electron app's configurations are handled in the package.json file."
- electron-builder config gains an `overwolf` sub-block, e.g. sample: `"build": { ..., "overwolf": { "disableAdOptimization": false } }`.

---

## 2. Declaring native packages in package.json

Verbatim from sample README + package.json:

```json
{
  "overwolf": {
    "packages": [
      "gep",
      "overlay"
    ]
  }
}
```
"In order to add more/remove certain ow-electron 'packages' from the project, simply edit the `overwolf.packages` array in the package.json file."

Known package names (from `ow-electron.d.ts` in the npm package, v39.6.1):
```typescript
type PackageName = 'gep' | 'overlay' | 'recorder' | 'utility' | string;
```
(`crn` also exists per the packages-types repo: modules are `crn.d.ts`, `gep.d.ts`, `overlay.d.ts`, `packages.d.ts`, `recorder.d.ts`, `utility.d.ts`.)

Runtime model: the declared packages are **downloaded/updated by the Overwolf package manager (`owepm`) at runtime**, not bundled by npm. FAQ: "Every few hours your app will check for a new version of the packages." â€¦ "When a new package is installed, the previous version of the same package is deleted." â€¦ "Yes, you app will need to restart in order for the package to update."

---

## 3. The exact GEP JS API

### 3.1 Entry point â€” the package manager (`app.overwolf.packages`)
From the **built-in typings shipped inside `@overwolf/ow-electron@39.6.1`** (`ow-electron.d.ts`, fetched from unpkg â€” authoritative):

```typescript
import { App, BrowserWindow, Event } from 'electron';

declare namespace overwolf {
  interface OverwolfApp extends App {
    overwolf: OverwolfApi;   // app.overwolf
  }
  interface OverwolfApi {
    disableAnonymousAnalytics(): void;   // call before app.ready
    disableAdsOptimization(): void;
    disableAdsFPD(): void;
    isCMPRequired(): Promise<boolean>;
    openCMPWindow(options?: CMPWindowOptions): Promise<void>;
    openAdPrivacySettingsWindow(options?: CMPWindowOptions): Promise<void>;
    packages: overwolf.packages.OverwolfPackageManager;   // <-- the package manager
    generateUserEmailHashes(email: string): EmailHashes;
    setUserEmailHashes(emailHashes?: EmailHashes): void;
    readonly phasePercent: number;
    readonly utmParams: any;
  }

  namespace packages {
    interface PackageInfo { name: string; version: string; }
    type PendingUpdatesResult = { hasPendingUpdate: boolean; details: PackageInfo[]; };

    interface OverwolfPackageManager extends NodeJS.EventEmitter {
      on(eventName: 'crashed', listener: (event: Event, canRecover: boolean) => void): this;
        // event.preventDefault() prevents auto re-launch of the crashed package
      on(eventName: 'ready', listener: (event: Event, packageName: PackageName, version: string) => void): this;
      on(eventName: 'package-update-pending', listener: (event: Event, info: PackageInfo[]) => void): this;
      on(eventName: 'updated', listener: (event: Event, packageName: string, version: string) => void): this;
      on(eventName: 'failed-to-initialize', listener: (event: Event, packageName: PackageName) => void): this;
      relaunch(): void;                       // force all pending package updates
      hasPendingUpdates(): PendingUpdatesResult;
      readonly logsFolderPath: string;
      readonly gep: packages.OverwolfGameEventPackage;  // available once 'ready' fired
    }
  }
}
```

### 3.2 The GEP package interface (verbatim, same `ow-electron.d.ts`; identical shape in `@overwolf/ow-electron-packages-types` `modules/gep.d.ts`)

```typescript
export interface GepGameLaunchEvent { enable: () => void; }

interface OverwolfGameEventPackage extends NodeJS.EventEmitter {
  getFeatures(gameId: number): Promise<string[]>;
  setRequiredFeatures(gameId: number, features: string[] | undefined): Promise<void>;
  getSupportedGames(): Promise<{ name: string; id: number; }[]>;
  getInfo(gameId: number): Promise<any>;

  on(eventName: 'new-info-update',
     listener: (event: Event, gameId: number, data: gep.InfoUpdate) => void): this;
  on(eventName: 'new-game-event',
     listener: (event: Event, gameId: number, data: gep.GameEvent) => void): this;
  on(eventName: 'game-detected',
     listener: (event: GepGameLaunchEvent, gameId: number, name: string, ...args: any[]) => void): this;
     // Call event.enable() to start GEP for this game
  on(eventName: 'game-exit',
     listener: (event: Event, gameId: number, gameName: string, pid: number,
                processName: string, processPath: string, commandLine: string) => void): this;
  on(eventName: 'elevated-privileges-required',
     listener: (event: Event, gameId: number, name: string, pid: number) => void): this;
     // "If this fires, it means the app must also run as administrator in order for Game Events to be detected."
  on(eventName: 'error',
     listener: (event: Event, gameId: number, error: string, ...args: any[]) => void): this;
}

namespace gep {
  interface GameEvent  { gameId: number; feature: string; key: string; value: any; }
  interface InfoUpdate extends GameEvent { category: string; }
}
```

Key precision points:
- **`setRequiredFeatures(gameId, features)` takes a gameId first** â€” the abbreviated docs snippet `app.overwolf.packages.gep.setRequiredFeatures(features)` omits it; the real signature (typings + sample) is `(gameId: number, features: string[] | undefined)`. Passing `null`/`undefined` subscribes to **all** features (the official sample does `await this.gepApi.setRequiredFeatures(gameId, null);`).
- Docs (GEP intro, dev.overwolf.com/ow-electron/live-game-data-gep/live-game-data-gep-intro/): "You **must** set the required Game Features for your app before GEP beings to work." and "The longer it takes between starting the game and when GEP is registered, the greater the chances of data issues".
- **There is NO `registerGames` method on the GEP package.** In the official sample, `registerGames(gepGamesId: number[])` is a helper method of the sample's own `GameEventsService` class that merely stores the ids and gates the `game-detected` handler. (`registerGames(filter)` IS a real method â€” but on the **overlay** package; see Â§5.)

### 3.3 Verbatim official sample code (`src/browser/services/gep.service.ts`, repo `overwolf/ow-electron-packages-sample`, MIT)

```typescript
import { app as electronApp } from 'electron';
import { overwolf } from '@overwolf/ow-electron'
import EventEmitter from 'events';

const app = electronApp as overwolf.OverwolfApp;

export class GameEventsService extends EventEmitter {
  private gepApi: overwolf.packages.OverwolfGameEventPackage;
  private activeGame = 0;
  private gepGamesId: number[] = [];

  constructor() {
    super();
    this.registerOverwolfPackageManager();
  }

  public registerGames(gepGamesId: number[]) {
    this.emit('log', `register to game events for `, gepGamesId);
    this.gepGamesId = gepGamesId;
  }

  public async setRequiredFeaturesForAllSupportedGames() {
    await Promise.all(this.gepGamesId.map(async (gameId) => {
      this.emit('log', `set-required-feature for: ${gameId}`);
      await this.gepApi.setRequiredFeatures(gameId, null);
    }));
  }

  public async getInfoForActiveGame(): Promise<any> {
    if (this.activeGame == 0) { return 'getInfo error - no active game'; }
    return await this.gepApi.getInfo(this.activeGame);
  }

  private registerOverwolfPackageManager() {
    app.overwolf.packages.on('ready', (e, packageName, version) => {
      if (packageName !== 'gep') { return; }
      this.emit('log', `gep package is ready: ${version}`);
      this.onGameEventsPackageReady();
      this.emit('ready');
    });
  }

  private async onGameEventsPackageReady() {
    this.gepApi = app.overwolf.packages.gep;
    this.gepApi.removeAllListeners();

    // To check if the game is running in elevated mode, use `gameInfo.isElevate`
    this.gepApi.on('game-detected', (e, gameId, name, gameInfo) => {
      if (!this.gepGamesId.includes(gameId)) {
        this.emit('log', 'gep: skip game-detected', gameId, name, gameInfo.pid);
        return;   // Stops the GEP Package from connecting to the game
      }
      this.emit('log', 'gep: register game-detected', gameId, name, gameInfo);
      e.enable();
      this.activeGame = gameId;
      // in order to start receiving event/info setRequiredFeatures should be set
    });

    // (sample comment: "undocumented ... event to track game-exit from the gep api")
    //@ts-ignore
    this.gepApi.on('game-exit', (e, gameId, processName, pid) => {
      console.log('gep game exit', gameId, processName, pid);
    });

    // **Note** - This fires AFTER `game-detected`
    this.gepApi.on('elevated-privileges-required', (e, gameId, ...args) => {
      this.emit('log', 'elevated-privileges-required', gameId, ...args);
    });

    this.gepApi.on('new-info-update', (e, gameId, ...args) => {
      this.emit('log', 'info-update', gameId, ...args);
    });

    this.gepApi.on('new-game-event', (e, gameId, ...args) => {
      this.emit('log', 'new-event', gameId, ...args);
    });

    this.gepApi.on('error', (e, gameId, error, ...args) => {
      this.emit('log', 'gep-error', gameId, error, ...args);
      this.activeGame = 0;
    });
  }
}
```
(Note: `game-exit` was "undocumented" at the time the sample was written but is now fully typed in both typings files with the 7-arg signature quoted in Â§3.2. Changelog for 39.6.0 also added "Async callback support for GEP game detection events".)

### 3.4 Game ID enums (typings package subpath imports)
Verbatim from `src/browser/application.ts` of the sample:
```typescript
import { GameInfo, GameLaunchEvent } from '@overwolf/ow-electron-packages-types';
import { kGameIds } from "@overwolf/ow-electron-packages-types/game-list";
import { kGepSupportedGameIds } from '@overwolf/ow-electron-packages-types/gep-supported-games';

gepService.registerGames([ kGepSupportedGameIds.TeamfightTactics ]);
...
this.overlayService.registerToGames([ kGameIds.LeagueofLegends, kGameIds.TeamfightTactics, ... ]);
```
`gep-supported-games.d.ts` (typings repo, main branch) is `export const enum kGepSupportedGameIds` with ~58 entries incl. **`VALORANT = 21640`**, `ApexLegends = 21566`, `CS2 = 22730`, `LeagueofLegends = 5426`, `Fortnite = 21216`, `MarvelRivals = 24890`, `Dota2 = 7314`, `RocketLeague = 10798`, `Overwatch = 10844`, `PUBG = 10906`, `EscapeFromTarkov = 21634`, etc. (Same 21640 Valorant id as the classic platform â€” matches the insights.gg V3 finding in project memory.)

### 3.5 GEP environments (PROD vs DEV) â€” per-game rollout
- Docs: "GEP for Overwolf Electron is currently being rolled out on a per game basis"; "not all games are available in the PROD environment".
- DEV environment switch (verbatim): `--owepm-packages-url=https://electronapi-qa.overwolf.com/packages` â€” command-line arg to your app; "After the game has been moved to _PROD_, you **need** to remove the command line argument." "Contact DevRel before moving to production."
- Current environment list (official "Electron Games Support" Trello card `trello.com/c/nqq4eYPg`, linked from docs; the docs page table at /ow-electron/live-game-data-gep/supported-environment/ is JS-rendered):
  - **Prod (17):** LOL, Rocket League, TFT, LOL Launcher - LEP, Diablo 4, Dota2, Apex Legends, Overwatch 2, Fortnite, CS 2, **Valorant**, Marvel Rivals, Halo infinite, Warframe, POE 1, POE 2, Hearthstone.
  - **Dev (29):** FF XIV, Minecraft, WoW, Genshin, BG3, Palworld, Helldivers 2, The Finals, Deadlock, Roblox, R6, New World, MTGA, Supervive, etc. â€” "All games in dev are ready to be transferred to prod on request."
  - **In Progress (13):** COD titles, PUBG, Escape from Tarkov, etc.
- **Valorant is in PROD for ow-electron GEP** â€” no QA URL flag needed.

---

## 4. Development vs production constraints; licensing

### 4.1 Development (does GEP work without approval?)
- **Stable line (â‰¤ 39.6.1, current `latest`):** No credential mechanism exists/is documented; the official sample's README instructions are just `npm run build && npm run start` (`ow-electron .`) with zero signup steps, and the GEP docs only require the QA URL flag for DEV-environment games. In practice the gaming packages download and run for unpackaged dev apps. (Valorant being PROD means no flag needed at all.)
- **New credential-gated "Dev Mode" (beta line):** Official Dev Mode page (dev.overwolf.com/ow-electron/guides/dev-tools/dev-mode): Dev Mode is "a path in the package manager (`owepm`) that skips most production integrity checks. It is built for local development, before your app is signed and packaged." and "Dev mode can't activate on a distributed or packaged app."
  - Credentials REQUIRED: "If `OW_CLI_EMAIL`, `OW_CLI_API_KEY`, or `OW_DEV_KEY` are all absent, dev mode credential verification fails and gaming packages don't load." Options: (A) env vars `OW_CLI_EMAIL` + `OW_CLI_API_KEY` (API key from the Overwolf Developer Console); (B) `ow config` CLI writing `~/.ow/credentials`; (C) temporary dev token `OW_DEV_KEY`.
  - With valid credentials "a dev-mode build runs the full gaming package set: GEP (game events), Overlay, Recorder." Without: "the app runs but the gaming packages don't load."
  - **Minimum versions: `ow-electron-builder@26.9.0-beta.2` + `ow-electron@39.8.10-beta.9`; Windows only.** Changelog 39.8.10 confirms: "Dev mode support via `OW_CLI_EMAIL`, `OW_CLI_API_KEY`, or temporary `OW_DEV_KEY`"; builder 26.9.0: "Overwolf signing during build with `OW_CLI_EMAIL`, `OW_CLI_API_KEY`, `OW_BUILD_KEY`" and "ASAR integrity validation enabled by default on Windows".
  - Getting Console credentials implies an approved developer account (see below). **Plan for the credential-gated model** â€” it is clearly where the platform is heading once 39.8.x goes stable.

### 4.2 Shipping publicly
- Whitelisting (Phase 1, verbatim): "Using Overwolf APIs requires your app idea to be whitelisted. Whitelisting is only granted to ideas submitted and approved via the App proposal process." Apps are classified public (appstore) or private â€” but "Overwolf currently doesn't approve private apps." Public apps "must have at least one desktop window indicating the app is running" (no faceless background processes).
- Release gating (Phase 3): DevRel QA review cycle; **code signing with your own cert from a trusted CA is mandatory** for the appstore and strongly recommended for self-hosting (FAQ: "with ow-electron you will need to provide your own code signing certificate from a trusted Certificate Authority"); CMP (consent management) integration required; public ToS + Privacy Policy URLs required in the installer. "Electron apps are only available in the Web based app store." Distribution is otherwise free-form (FAQ: "you are able to share your app with anyone you like as well as host the app in any location you prefer").
- Build-time Overwolf signing (beta line): `OW_BUILD_KEY` during `ow-electron-builder` runs.

### 4.3 Licensing terms
- npm packages: MIT. Runtime services: proprietary/closed-source (FAQ quote in Â§1).
- **Overwolf Developer Terms and Conditions** (legal.overwolf.com/docs/overwolf/developers/developer-terms/) applies to Platform API use. Key clauses (verbatim): Â§9 "All Advertisements displayed or distributed on your Applications shall be generated exclusively through Overwolf's proprietary advertising platform." Â§6.2 ad Revenue Share = "thirty percent (30%) of the Revenues actually received through the advertising platform" to Overwolf (dev keeps 70%); subscription share 15% to Overwolf. Â§2.4 Overwolf "may at any time, without notice â€¦ cease to provide any or all Applications with access to the Platform API." Â§15.2 Overwolf "may immediately terminate or suspend these Terms â€¦ at its sole discretion at any time, for any reason or no reason at all". Monetization policy: Overwolf "will only approve apps that integrate and use Overwolf ads, Overwolf subscriptions, or both" (no 3rd-party monetization).
- Analytics is on by default; opt-out API `app.overwolf.disableAnonymousAnalytics()` (must be called before `app.ready`).

---

## 5. Overlay package (optional for us) and Recorder package (we won't use)

### 5.1 Overlay (`'overlay'`) â€” `IOverwolfOverlayApi` (typings `modules/overlay.d.ts`, verbatim signatures)
```typescript
interface IOverwolfOverlayApi extends EventEmitter {
  createWindow(options: OverlayWindowOptions): Promise<OverlayBrowserWindow>;
  registerGames(filter: GamesFilter);                 // which games to inject into
  getActiveGameInfo(): ActiveGameInfo | undefined;
  getAllWindows(): OverlayBrowserWindow[];
  fromWebContents(webContents: WebContents): OverlayBrowserWindow | null;
  fromBrowserWindow(browserWindow: BrowserWindow): OverlayBrowserWindow | null;
  requestGameInjection(classId: number): Promise<void>;
  hotkeys: IOverlayHotkeys;   // register/update/unregister/unregisterAll/all
  readonly version: string;
  enterExclusiveMode(options?: ExclusiveInputOptions): void;
  exitExclusiveMode(): void;
  installHighElevationHelper?(): Promise<void>;
  isHighElevationHelperInstalled?(): Promise<boolean>;
  takeScreenshot(filePath: string, format?: 'jpg' | 'bmp'): Promise<void>;

  on(eventName: 'game-launched', listener: (event: GameLaunchEvent, gameInfo: GameInfo) => void): this;
  on(eventName: 'game-exit', listener: (gameInfo: GameInfo, wasInjected: boolean) => void): this;
  on(eventName: 'game-injected', listener: (gameInfo: GameInfo) => void): this;
  on(eventName: 'game-injection-error', listener: (gameInfo: GameInfo, error: string, ...args: any[]) => void): this;
  on(eventName: 'game-focus-changed', listener: (window: GameWindowInfo, gameInfo: GameInfo, focus: boolean) => void): this;
  on(eventName: 'game-window-changed', listener: (window: GameWindowInfo, gameInfo: GameInfo, reason?: GameWindowUpdateReason) => void): this;
  on(eventName: 'game-input-interception-changed', listener: (info: GameInputInterception) => void): this;
  on(eventName: 'game-input-exclusive-mode-changed', listener: (info: GameInputInterception) => void): this;
  on(eventName: 'error', listener: (...args: any[]) => void): this;
}
```
`GameLaunchEvent` has `inject()` and `dismiss()`; the sample's flow: `overlayService.on('injection-decision-handling', (event, gameInfo) => event.inject())` after `registerGames`. FAQ confirms: "Using the Overwolf Electron Overlay API, you can call the registerGames() method, which accepts gameIds: number[]." Supports Standard mode (visible-cursor games) and Exclusive mode (FPS), OOPO (out-of-process overlay), HDR, hotkeys. Note the FAQ elevation warning: "When launching your app, check the admin privileges using the game-launched event from the Overlay package." **There is no `registerOverlay` method â€” game registration is `overlay.registerGames(filter)`.**

### 5.2 Recorder (`'recorder'`) â€” noting only (OBS does our capture)
`IOverwolfRecordingApi` (typings `modules/recorder.d.ts` + reference overview): two modes â€” Standard ("all video is recorded from start to finish when stopped and is saved to storage") and Replay ("records based on a provided time frame (cached sliding buffer)" â€” "Record a buffer of X seconds to memory without saving to disk" then capture on demand). Capture from display or game; any audio device or game-only audio; encoder matrix (x264, NVENC H.264/HEVC, AMD AMF incl. AV1, Intel QuickSync) with rate-control/preset types; `RecordingOptions { filePath, fileFormat, audioTrack, autoShutdownOnGameExit, split }`, `SplitOptions { enableManual, maxTimeSecond, maxBySizeMB }`, `ReplayOptions { bufferSecond, ... }`; `CaptureSettingsBuilder` pattern; perf stats (CPU/mem, disk, FPS, dropped frames). Docs example: `await app.recorder.startRecording({ filePath: 'C:/Videos/gameplay', audioTrack: 1 })`. Recorder package version stream ~`0.32.37+`. It is effectively Overwolf's OBS-replacement; irrelevant when OBS owns capture, but its existence confirms replay-buffer-style workflows are natively supported on this stack if ever needed.

### 5.3 Other packages
- `utility` (`'utility'`): `trackGames(filter: GamesFilter): Promise<void>`, `scan(filter?): Promise<InstalledGameInfo[]>`, events `'game-launched'` / `'game-exit'` `(gameInfo: GameInfo)` â€” lightweight game-launch tracking WITHOUT injection/GEP; useful to auto-detect any game start.
- `crn`: "Content Recommendation Notification" APIs (notification settings management).

---

## 6. Outbound localhost WebSocket from an ow-electron app

**Nothing blocks it.** Findings:
- ow-electron is a fork of stock Electron; the main process is full Node.js. An outbound `ws://127.0.0.1:<port>` client (e.g. `ws` npm package) from the **main process** is plain Node networking â€” no Overwolf-imposed network policy is documented anywhere (docs, FAQ, changelog), and no GitHub/forum issues reporting blocked localhost connections in ow-electron exist.
- The only documented network-related additions are Overwolf's own services (package downloads every few hours, ads/analytics endpoints, electron-updater endpoint) â€” additive, not restrictive.
- If you connect from a **renderer** instead, the only constraints are the standard web-platform ones you control yourself (your page's CSP `connect-src`, mixed-content if you ever host the UI over https â€” `ws://localhost` from a `file://`/`app://` window is fine). Recommendation: put the WS client in the main process next to the GEP listeners anyway (single event pipeline, survives window lifecycle, no CSP involved).
- Windows loopback is unrestricted for desktop apps (the UWP loopback exemption problem does not apply). Precedent: Overwolf's ecosystem itself is built around localhost WS patterns (classic API even ships `overwolf.web.createWebSocket()` whose stated advantage is "Our web sockets by-pass cert checks for localhost WSS servers like LCU (league of legends)") â€” plain `ws://` to the OBS plugin needs none of that.

---

## 7. The CLASSIC alternative â€” Overwolf native platform app

### 7.1 API surface (`overwolf.games.events`, docs reference)
- `overwolf.games.events.setRequiredFeatures(features: string[], callback: (result: SetRequiredFeaturesResult) => void)` (since 0.93) â€” note: **no gameId param** (applies to the current game); docs: "it's important to wait for the **success** status"; recommended retry loop ~3s intervals.
- `overwolf.games.events.getInfo(callback: (result: GetInfoResult) => void)` (0.95).
- `overwolf.games.events.onNewEvents.addListener(cb)` (0.96) â€” "Fired when there are new game events with a JSON object of events information"; payload `NewGameEvents = { events: GameEvent[] }`, e.g. Valorant: `{"events":[{"name": "match_start","data":""}]}`.
- `overwolf.games.events.onInfoUpdates2.addListener(cb)` (0.96) â€” payload `InfoUpdates2Event` with `info` + `feature`, e.g. `{"info":{"me":{"player_name":"Doom#5339"}},"feature":"me"}`.
- `overwolf.games.events.onError.addListener(cb)` (0.78) â€” `{ reason: string }`.
- Manifest requirements (manifest.json reference): `"game_targeting": { "type": "dedicated", "game_ids": [21640] }`, `"game_events": [21640]` ("game IDs for which real-time game events are required"), `"launch_events": [{ "event": "GameLaunch", "event_data": { "game_ids": [21640] } }]`, `"permissions": ["GameInfo"]`.

### 7.2 Same event payloads?
**Same underlying GEP data, different envelope.** Both platforms are fed by the same Game Events Provider; game pages (e.g. Valorant, game 21640: features `gep_internal`, `me`, `game_info`, `match_info`, `kill`, `death`; keys like `kill_feed`, `scoreboard_N`, `round_phase` âˆˆ shopping/combat/end/game_end, `match_start`/`match_end`) are shared docs for both. Delivery shape differs:
- Classic: `onNewEvents` â†’ `{ events: [{ name, data }] }`; `onInfoUpdates2` â†’ `{ info: { <category>: { <key>: <value> } }, feature }`.
- ow-electron: `new-game-event` â†’ `(event, gameId, { gameId, feature, key, value })`; `new-info-update` â†’ `(event, gameId, { gameId, feature, category, key, value })` (flattened, one callback per item, gameId included since one app can watch multiple games).
Same feature names are passed to `setRequiredFeatures` on both. Caveat: per-game availability differs â€” classic supports the full GEP catalog; ow-electron is per-game rollout (Valorant already PROD, see Â§3.5).

### 7.3 Can a classic app run locally as an unpacked dev app without store approval?
**Not without whitelisting.** Verbatim: "To develop, load or run unpacked or unreleased apps, you have to get whitelisted first." (setting-up-dev-environment). Dev tools: "Chrome Developer Tools are disabled by default in Overwolf â€¦ Access is restricted to **developers** who have been approved by Overwolf." Loading unpacked = Overwolf client â†’ settings â†’ Development Options â†’ "Load unpacked extension"; a non-whitelisted account gets an "Unauthorized App" error. So classic needs the same app-proposal approval as ow-electron â€” PLUS the end user must have the whole Overwolf client installed (apps run inside the client/CEF), and distribution to users is only via the Overwolf appstore after review. Useful classic-side tooling that partially carries over conceptually: the GEP Simulator app and Events Recorder/Player (ERP) for replaying recorded event streams without playing the game.

### 7.4 Comparison and recommendation

| Criterion | ow-electron companion app | Classic Overwolf app |
|---|---|---|
| Runtime | Standalone Electron 39 fork; full Node in main proc | Runs inside Overwolf client (CEF); user must install Overwolf |
| GEP access | `app.overwolf.packages.gep`, Promise-based, multi-game, `game-detected`/`enable()` handshake | `overwolf.games.events`, callback-based, current-game only |
| Valorant (21640) | PROD âœ… | PROD âœ… (mature) |
| Localhost WS to OBS plugin | Trivial (`ws` in main process); nothing blocks | Possible (browser `WebSocket` / `overwolf.web`), but app only runs when the OW client runs |
| Dev without approval | Stable line: sample runs as-is today; beta 39.8.10+/builder 26.9.0+: requires Console credentials (`OW_CLI_EMAIL`+`OW_CLI_API_KEY` or `OW_DEV_KEY`) | Hard no â€” whitelisted account required even to load unpacked |
| Ship publicly | App proposal + whitelisting + QA + own code-signing cert + CMP; distribute anywhere (own site OK) | App proposal + QA + appstore only; users need Overwolf client |
| Terms | OW ads exclusivity, 70/30 ads split, API access revocable at will | Same developer terms |
| Typings/sample | `@overwolf/ow-electron-packages-types@1.1.4`, official `ow-electron-packages-sample` | `@overwolf/types` + classic sample-app |

**Recommendation: build the companion on ow-electron.** Rationale: (1) it is a normal Electron app â€” the GEPâ†’WebSocket forwarder is ~200 lines in the main process (copy the sample's `GameEventsService`, replace its `emit('log', ...)` with a `ws` client sending JSON to the OBS plugin's `ws://127.0.0.1:<port>`); (2) OBS users should not be forced to install the full Overwolf client with its ad platform â€” an ow-electron app is self-distributed and standalone; (3) Valorant GEP is PROD on ow-electron, and payloads carry the same feature/key data the project already reverse-engineered from insights.gg V3 (itself an ow-electron-stack product); (4) the classic path gives no approval-shortcut anyway (unpacked loading also requires whitelisting) while adding the client dependency and appstore-only distribution. Plan for: submitting the app proposal early (whitelisting unlocks Console credentials for the credential-gated Dev Mode that arrives with ow-electron 39.8.10+), pinning `@overwolf/ow-electron@39.6.1` + `@overwolf/ow-electron-builder@26.8.5` initially, `"overwolf": { "packages": ["gep"] }` only (add `"overlay"` later if in-game notifications are wanted), and budgeting for a code-signing cert + CMP + ToS/Privacy URLs before any public release.
