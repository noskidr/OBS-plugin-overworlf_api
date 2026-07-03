# OBS Plugin Engineering Research â€” patterns to copy (verified against source, 2026-07-03)

All line numbers refer to the default branch (`master`/`main`) of each repo as fetched 2026-07-03. Everything below was read from raw source, not from memory.

---

## 1. obs-plugintemplate â€” CURRENT state (github.com/obsproject/obs-plugintemplate, last commits 2025-12-09)

### 1.1 CMakePresets.json (verbatim facts)
File: `https://github.com/obsproject/obs-plugintemplate/blob/master/CMakePresets.json`
- `"version": 8`, `"cmakeMinimumRequired": {"major": 3, "minor": 28, "patch": 0}`
- Configure presets: `template` (hidden base), `macos`, `macos-ci`, `windows-x64`, `windows-ci-x64`, `ubuntu-x86_64`, `ubuntu-ci-x86_64`
- Build presets (same names): `macos`, `macos-ci`, `windows-x64`, `windows-ci-x64`, `ubuntu-x86_64`, `ubuntu-ci-x86_64` â€” all `"configuration": "RelWithDebInfo"`
- The hidden base preset is where the two feature flags default OFF:
```json
{
  "name": "template",
  "hidden": true,
  "cacheVariables": {
    "ENABLE_FRONTEND_API": false,
    "ENABLE_QT": false
  }
}
```
- Windows preset (the one you asked about):
```json
{
  "name": "windows-x64",
  "displayName": "Windows x64",
  "inherits": ["template"],
  "binaryDir": "${sourceDir}/build_x64",
  "condition": {"type": "equals", "lhs": "${hostSystemName}", "rhs": "Windows"},
  "generator": "Visual Studio 17 2022",
  "architecture": "x64,version=10.0.22621",
  "warnings": {"dev": true, "deprecated": true}
}
```
`windows-ci-x64` inherits it and adds `"CMAKE_COMPILE_WARNING_AS_ERROR": true`.
- macOS: Xcode generator, `CMAKE_OSX_DEPLOYMENT_TARGET 12.0`, `CMAKE_OSX_ARCHITECTURES "arm64;x86_64"`, binaryDir `build_macos`. Ubuntu: Ninja, `CMAKE_BUILD_TYPE RelWithDebInfo`, binaryDir `build_x86_64`.

### 1.2 buildspec.json (exact current pins)
File: `https://github.com/obsproject/obs-plugintemplate/blob/master/buildspec.json`
```json
"dependencies": {
  "obs-studio": { "version": "31.1.1", "baseUrl": "https://github.com/obsproject/obs-studio/archive/refs/tags", "label": "OBS sources",
    "hashes": { "macos": "39751f06â€¦", "windows-x64": "2c8427c1â€¦" } },
  "prebuilt":   { "version": "2025-07-11", "baseUrl": "https://github.com/obsproject/obs-deps/releases/download", "label": "Pre-Built obs-deps",
    "hashes": { "macos": "495687e6â€¦", "windows-x64": "c8c642c1â€¦" } },
  "qt6":        { "version": "2025-07-11", "baseUrl": "https://github.com/obsproject/obs-deps/releases/download", "label": "Pre-Built Qt6",
    "hashes": { "macos": "d3f5f04bâ€¦", "windows-x64": "0e76bf05â€¦" },
    "debugSymbols": { "windows-x64": "11b7be92â€¦" } }
},
"platformConfig": { "macos": { "bundleId": "com.example.plugintemplate-for-obs" } },
"name": "plugintemplate-for-obs", "displayName": "Plugin Template for OBS",
"version": "1.0.0", "author": "Your Name Here", "website": "https://example.com", "email": "me@example.com"
```
Schema fields: `dependencies.{obs-studio|prebuilt|qt6}.{version,baseUrl,label,hashes.{macos,windows-x64},debugSymbols.windows-x64}`, `platformConfig.macos.bundleId`, `name`, `displayName`, `version`, `author`, `website`, `email`.
Version context: obs-deps release `2025-07-11` = **Qt 6.8.3** (`deps.qt/qt6.ps1` at tag 2025-07-11: `$Version = '6.8.3'`). Latest obs-deps is `2026-06-25` (Qt 6.11.1 on obs-deps master). Latest OBS Studio release is **32.1.2** (2026-04-21) â€” the template still pins 31.1.1, but its CI got an OBS-32 fix ("CI: Add libsimde-dev for OBS 32 on Ubuntu", Sep 2025). Deps are downloaded into `${CMAKE_SOURCE_DIR}/.deps/` and `cmake/common/buildspec_common.cmake` verifies them via `share/obs-deps/VERSION` in `CMAKE_PREFIX_PATH`; on Windows it also **configures and builds the `obs-frontend-api` target from the downloaded obs-studio sources in both Debug and Release** (`--target obs-frontend-api`), which is what `find_package(obs-frontend-api)` later imports.

### 1.3 ENABLE_FRONTEND_API / ENABLE_QT â€” exact CMakeLists.txt lines
File: `https://github.com/obsproject/obs-plugintemplate/blob/master/CMakeLists.txt` (entire file is 39 lines):
```cmake
cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" OFF)
option(ENABLE_QT "Use Qt functionality" OFF)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/plugin-main.c)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
```
How to turn them on (per template wiki "CMake Build System Options"): either `cmake --preset windows-x64 -DENABLE_FRONTEND_API:BOOL=ON -DENABLE_QT:BOOL=ON`, or (recommended for a dock plugin, and what the wiki says is "correct" for frontend plugins) edit the `template` preset in `CMakePresets.json` to `"ENABLE_FRONTEND_API": true, "ENABLE_QT": true`. Also rename `src/plugin-main.c` to `.cpp` in `target_sources` for a Qt/C++ plugin (vertical-canvas does exactly this).

### 1.4 GitHub Actions shipped
Files under `.github/workflows/`: `push.yaml`, `pr-pull.yaml`, `dispatch.yaml`, `build-project.yaml` (reusable, `workflow_call`), `check-format.yaml`.
- `pr-pull.yaml`: `on: workflow_dispatch` + `pull_request: {paths-ignore: ['**.md'], branches: [master, main], types: [opened, synchronize, reopened]}`; concurrency cancel-in-progress; runs check-format + build-project.
- `push.yaml`: `on: push: branches: [master, main, 'release/**'], tags: ['*']`. Tag pushes matching `+([0-9]).+([0-9]).+([0-9])` (or `-beta*/-rc*`) create a **draft GitHub release** (softprops/action-gh-release pinned by SHA) with renamed artifacts + `CHECKSUMS.txt` (sha256).
- `build-project.yaml` job `check-event` computes: `pull_request` â†’ `package:false, config:RelWithDebInfo` (package/codesign flip to true if PR has label **"Seeking Testers"**); `push` â†’ `package:true`, and if ref matches semver â†’ `notarize:true, config:Release`.
- Runners: `macos-15` (switches to Xcode 16.1), `ubuntu-24.04`, `windows-2022` (pwsh).
- **Artifact naming** (upload-artifact@v4):
  - Windows: `${pluginName}-${pluginVersion}-windows-x64-${commitHash}` where commitHash = `${GITHUB_SHA:0:9}`; contents `release/<name>-<version>-windows-x64*.*` (zip; exe installer when packaged).
  - macOS: `â€¦-macos-universal-${hash}` (tar.xz / pkg) plus `â€¦-dSYMs` artifact when config==Release.
  - Ubuntu: `â€¦-ubuntu-24.04-x86_64-${hash}` (tar.xz/deb/ddeb), a `â€¦-sources-${hash}` tarball, and `â€¦-dbgsym` ddeb when packaged.
- Windows CI build steps (`.github/scripts/Build-Windows.ps1` lines 50-80): `cmake --preset windows-ci-x64` â†’ `cmake --build --preset windows-x64 --config RelWithDebInfo --parallel` â†’ `cmake --install build_x64 --prefix release/RelWithDebInfo`.
- Minimums (README "Supported Build Environments" table): Windows = **Visual Studio 17 2022**, CMake **3.30.5** (win/mac), Xcode 16.0, Ubuntu 24.04 CMake 3.28.3 + ninja-build + pkg-config + build-essential. Hard floor from CMake files: **CMake â‰¥ 3.28** (`cmake_minimum_required(VERSION 3.28...3.30)` and presets `cmakeMinimumRequired` 3.28.0). Windows generator: **"Visual Studio 17 2022"**, arch `x64,version=10.0.22621` (Windows 11 SDK 22621 must be installed).

---

## 2. WebSocket SERVER inside a native OBS plugin

### 2.1 What obs-websocket actually uses (github.com/obsproject/obs-websocket, master = 5.7.4)
`CMakeLists.txt` (top):
```cmake
set(obs-websocket_VERSION 5.7.4)
find_package(Qt6 REQUIRED Core Widgets Svg Network)
find_package(nlohmann_json 3.11 REQUIRED)
find_package(qrcodegencpp REQUIRED)
find_package(Websocketpp 0.8 REQUIRED)
find_package(Asio 1.12.1 REQUIRED)
...
target_compile_definitions(obs-websocket PRIVATE ASIO_STANDALONE
  $<$<PLATFORM_ID:Windows>:_WEBSOCKETPP_CPP11_STL_>
  $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0603>)
target_link_libraries(obs-websocket PRIVATE OBS::libobs OBS::frontend-api OBS::websocket-api
  Qt::Core Qt::Widgets Qt::Svg Qt::Network nlohmann_json::nlohmann_json
  Websocketpp::Websocketpp Asio::Asio qrcodegencpp::qrcodegencpp)
```
So: **websocketpp + standalone Asio (no Boost), nlohmann::json, Qt Network only for auxiliary UI (ConnectInfo/IP display)**. Server internals (`src/websocketserver/WebSocketServer.cpp`): `websocketpp::server<websocketpp::config::asio>`; `_server.init_asio()` (line 37); handlers `set_validate/open/close/message_handler` (44-48); `_server.listen(websocketpp::lib::asio::ip::tcp::v4(), conf->ServerPort, errorCode)` (105); one dedicated thread `_serverThread = std::thread(&WebSocketServer::ServerRunner, this)` (123) running `_server.run()` (61); `_serverThread.join()` on stop (161). Incoming messages are NOT processed on the asio thread: `onMessage` copies the payload and dispatches to a **QThreadPool** â€” `_threadPool.start(Utils::Compat::CreateFunctionRunnable([hdl, payload, opCode, this]() { â€¦ json::parse(payload) â€¦ }))` (lines 347-411) â€” and request handlers then call `obs_frontend_*`/libobs directly from pool threads (see Â§5 for why that is safe).

### 2.2 Where the headers come from â€” obs-deps ships them (Windows + macOS)
`github.com/obsproject/obs-deps/deps.windows/` contains `60-asio.ps1`, `60-websocketpp.ps1`, `60-nlohmann-json.ps1`:
- `60-asio.ps1`: `Version = '1.32.0'`, copies `include/asio.hpp` + `include/asio/` into the deps `include/` output.
- `60-websocketpp.ps1`: `Version = '0.8.2'` (zaphoyd/websocketpp @ 56123c87), patched (`0001-update-minimum-cmake.patch`), CMake-installed with `-DENABLE_CPP11:BOOL=ON`.
- `60-nlohmann-json.ps1`: `Version = '3.11.3'`.
These are inside the **same "Pre-Built obs-deps" archive the template already downloads to `.deps/obs-deps-<version>-<arch>/`** per buildspec.json. Same versions present at the template-pinned tag `2025-07-11`.

**Proof a template plugin can consume them with ~10 lines of CMake** â€” occ-ai/obs-urlsource (an obs-plugintemplate-based plugin that vendors a websocketpp client) `cmake/FetchWebsocketpp.cmake`:
```cmake
if(WIN32 OR APPLE)
  # Windows and macOS are supported by the prebuilt dependencies
  file(READ "${CMAKE_SOURCE_DIR}/buildspec.json" buildspec)
  string(JSON version GET ${buildspec} dependencies prebuilt version)
  if(MSVC)
    set(arch ${CMAKE_GENERATOR_PLATFORM})
  elseif(APPLE)
    set(arch universal)
  endif()
  set(deps_root "${CMAKE_SOURCE_DIR}/.deps/obs-deps-${version}-${arch}")
  target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${deps_root}/include")
else()
  # Linux requires fetching the dependencies
  include(FetchContent)
  FetchContent_Declare(websocketpp URL https://github.com/zaphoyd/websocketpp/archive/refs/tags/0.8.2.tar.gz
    URL_HASH SHA256=6ce889d85ecdc2d8fa07408d6787e7352510750daa66b5ad44aacb47bea76755)
  ... 
  FetchContent_Declare(asio URL https://github.com/chriskohlhoff/asio/archive/asio-1-28-0.tar.gz ...)
endif()
```
and its `src/websocket-client.cpp` compiles with:
```cpp
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
```
(For a server use `<websocketpp/config/asio_no_tls.hpp>` + `websocketpp::server` and copy obs-websocket's Windows defines `_WEBSOCKETPP_CPP11_STL_` and `_WIN32_WINNT=0x0603`.)

### 2.3 Does OBS's Qt include Qt6WebSockets / Qt6Network on Windows?
From `obs-deps/deps.qt/qt6.ps1` (both at pinned tag 2025-07-11 and master):
```powershell
$QtComponents = @(
    'qtbase'
    'qtimageformats'
    'qtshadertools'
    'qtmultimedia'
    'qtsvg'
    'qttools'
)
```
- **Qt6WebSockets: NOT built, NOT shipped** (`qtwebsockets` absent). `QWebSocketServer` is a dead end unless you bundle your own Qt module binary-compatible with OBS's exact Qt build â€” fragile and not done by any known plugin.
- **Qt6Network: YES** â€” it is part of `qtbase` and nothing in the configure options disables the network feature (TLS backend: `-DFEATURE_schannel:BOOL=ON`, `-DINPUT_openssl:STRING=no` on Windows). Runtime proof: obs-websocket links `Qt::Network` and ships enabled in every official OBS Windows build, so `Qt6Network.dll` is present in `obs-studio/bin/64bit` of every OBS 28+ install.

### 2.4 Evidence-based recommendation (localhost-only, JSON text frames, 1-2 clients)
**Recommended: vendor websocketpp 0.8.2 + standalone asio exactly as obs-websocket does, using the headers already in the template's `.deps` prebuilt bundle (copy occ-ai's `FetchWebsocketpp.cmake`, switch to the server config).** Rationale:
1. Zero new dependency management on Windows/macOS â€” headers are already on disk in every template build (`60-asio.ps1`/`60-websocketpp.ps1` above), header-only, no DLLs to ship.
2. It is the exact stack running inside every OBS install today (obs-websocket 5.x) â€” battle-tested against the same compiler/runtime, and obs-deps carries the compatibility patch for modern CMake.
3. Thread model is simple and OBS-proven: one `std::thread` running `server.run()`, handlers marshal into OBS/Qt (see Â§5). No coupling of network I/O to the Qt main-thread event loop.
4. Known caveat: websocketpp is unmaintained (0.8.2, 2020) â€” acceptable here because obs-deps pins/patches it and the OBS project itself still ships it; asio 1.32.0 satisfies the `Asio 1.12.1` minimum.
- **Second choice â€” QTcpServer (Qt6Network) + hand-rolled RFC 6455**: viable since Qt6Network ships; everything can live on the Qt main thread (fine at 1-2 clients / a few msgs/sec); `QCryptographicHash::Sha1` + base64 covers `Sec-WebSocket-Accept`. Cost: you own the HTTP Upgrade parse, frame decode (masking mandatory from clients), fragmentation, ping/pong, close handshake â€” several hundred lines of protocol code with real interop risk vs. Electron/`ws` clients, for no dependency saving that matters (headers are already vendored). Choose only if a "no third-party code" constraint exists.
- **Rejected: raw-socket hand-rolled RFC6455 (no Qt)** â€” same protocol burden as above plus your own socket/event loop; and **Qt6WebSockets** â€” not shipped (2.3).

---

## 3. Real template-based plugins with Qt docks â€” exact files/lines to copy

### 3.1 Aitum/obs-vertical-canvas (built from obs-plugintemplate: has buildspec.json, CMakePresets.json, GitHub workflows; default branch `main`)
File `vertical-canvas.cpp` (https://github.com/Aitum/obs-vertical-canvas/blob/main/vertical-canvas.cpp):
- **(a) Dock creation via obs_frontend_add_dock_by_id** â€” lines 693-713 (inside `obs_module_post_load`, after loading config):
```cpp
const auto canvasDock = new CanvasDock(nullptr, main_window);
const QString title = QString::fromUtf8(obs_module_text("Vertical"));
const auto name = "VerticalCanvasDock";
obs_frontend_add_dock_by_id(name, title.toUtf8().constData(), canvasDock);
```
  Sub-docks per canvas at lines 1168-1179 (`"VerticalCanvasDockScenes"`, `"VerticalCanvasDockSources"`, `"VerticalCanvasDockTransitions"` â€” id must be unique; title reuses `obs_frontend_get_locale_string("Basic.Main.Scenes")`).
- **(b) Background thread â†’ Qt UI marshalling** â€” libobs output signal callbacks (fire on libobs threads) at lines 5627-5658:
```cpp
void CanvasDock::record_output_start(void *data, calldata_t *calldata)
{
    auto d = static_cast<CanvasDock *>(data);
    ...
    QMetaObject::invokeMethod(d, "OnRecordStart");
}
void CanvasDock::record_output_stop(void *data, calldata_t *calldata)
{
    const char *last_error = (const char *)calldata_ptr(calldata, "last_error");
    QString arg_last_error = QString::fromUtf8(last_error);
    const int code = (int)calldata_int(calldata, "code");
    auto d = static_cast<CanvasDock *>(data);
    ...
    QMetaObject::invokeMethod(d, "OnRecordStop", Q_ARG(int, code), Q_ARG(QString, arg_last_error));
}
```
  Same pattern with explicit queueing from the frontend-event callback: `QMetaObject::invokeMethod(it, "MainSceneChanged", Qt::QueuedConnection);` (line 129) and ~35 more occurrences (lines 178-226, 5167, 5633-5658â€¦). Signals hooked via `signal_handler_connect(rpsh, "saved", replay_saved, this)` (line 1163, replay-buffer `saved` signal) and global signals `signal_handler_connect(obs_get_signal_handler(), "source_rename"/"source_remove"/"source_destroy"/"source_save", â€¦)` (lines 1512-1518).
- **(c) Frontend event callback** â€” `frontend_event()` at line 133 handles `OBS_FRONTEND_EVENT_EXIT`, `OBS_FRONTEND_EVENT_SCRIPTING_SHUTDOWN` (save config + tear down docks), `SCENE_COLLECTION_CHANGING/CLEANUP`, `FINISHED_LOADING` (connect transition signals); registered in `obs_module_load` line 649: `obs_frontend_add_event_callback(frontend_event, nullptr);`, removed in `obs_module_unload` line 776.

### 3.2 exeldro/obs-source-dock
File `source-dock.cpp` (https://github.com/exeldro/obs-source-dock/blob/master/source-dock.cpp):
- `obs_module_load()` lines 500-521: `obs_frontend_add_save_callback(frontend_save_load, nullptr); obs_frontend_add_event_callback(frontend_event, nullptr); signal_handler_connect(obs_get_signal_handler(), "source_remove", source_remove, nullptr);` plus Tools-menu entry via `obs_frontend_add_tools_menu_qaction` + `obs_frontend_push_ui_translation(obs_module_get_string)` before constructing the Qt dialog.
- Dynamic dock creation from persisted data, line 203: `if (!obs_frontend_add_dock_by_id(title, title, tmp)) { delete tmp; continue; }` then grabs the QDockWidget wrapper via `static_cast<QDockWidget *>(tmp->parentWidget())` (line 208) to restore hidden/floating state.
- Dock removal: `obs_frontend_remove_dock(it->objectName().toUtf8().constData());` in `frontend_event` on `SCENE_COLLECTION_CLEANUP`/`EXIT` (line 436) and in the `source_remove` signal handler (line 492).

### 3.3 The dock API itself
`frontend/api/obs-frontend-api.h` (OBS master):
```c
/* takes QWidget for widget */
EXPORT bool obs_frontend_add_dock_by_id(const char *id, const char *title, void *widget);   // line 155
EXPORT void obs_frontend_remove_dock(const char *id);                                       // line 157
typedef void (*obs_frontend_event_cb)(enum obs_frontend_event event, void *private_data);   // line 162
EXPORT void obs_frontend_add_event_callback(obs_frontend_event_cb callback, void *private_data);
```
Implementation `frontend/OBSStudioAPI.cpp:336-380`: rejects duplicate ids (`IsDockObjectNameUsed`), does `OBSDock *dock = new OBSDock(main); dock->setWidget((QWidget *)widget); dock->setWindowTitle(QT_UTF8(title)); ... main->AddCustomDockWidget(d);` â€” i.e. it constructs QWidgets **directly on the calling thread â†’ UI-thread-only call** (call it from `obs_module_load`/`obs_module_post_load`/frontend event callbacks, all UI-thread). Available since **OBS 30.0.0** (absent in 29.1.3 header). Events useful to you (enum in obs-frontend-api.h lines 15-63): `OBS_FRONTEND_EVENT_RECORDING_STARTED/STOPPED/PAUSED/UNPAUSED`, `REPLAY_BUFFER_STARTED/SAVED/STOPPED`, `STREAMING_STARTED/STOPPED`, `FINISHED_LOADING`, `SCRIPTING_SHUTDOWN`, `EXIT`.
Related recording/replay API (same header, lines 184-199, 205-206, 242, 247-249): `obs_frontend_recording_start/stop/active/pause/paused`, `bool obs_frontend_recording_split_file(void)`, `bool obs_frontend_recording_add_chapter(const char *name)` (added **OBS 30.2.0**; works with Hybrid MP4 output), `obs_frontend_replay_buffer_start/save/stop/active`, `obs_output_t *obs_frontend_get_replay_buffer_output(void)`, `char *obs_frontend_get_current_record_output_path(void)`, `char *obs_frontend_get_last_recording(void)`, `char *obs_frontend_get_last_replay(void)`, `char *obs_frontend_get_last_screenshot(void)` (all `char*` returns must be `bfree`d).

---

## 4. Dynamic overlay rendering in a native source

### 4.1 Pattern (a): CPU-render â†’ gs_texture upload
Canonical in-tree example â€” the GDI+ text source itself, `plugins/obs-text/gdiplus/obs-text.cpp`:
- `TextSource::RenderText()` (lines 567-643): renders with GDI+ into a malloc'd BGRA buffer (`unique_ptr<uint8_t[]> bits(new uint8_t[size.cx * size.cy * 4]); Bitmap bitmap(size.cx, size.cy, 4 * size.cx, PixelFormat32bppARGB, bits.get()); Graphics graphics_bitmap(&bitmap); â€¦`), then uploads:
```cpp
if (!tex || (LONG)cx != size.cx || (LONG)cy != size.cy) {
    obs_enter_graphics();
    if (tex) gs_texture_destroy(tex);
    const uint8_t *data = (uint8_t *)bits.get();
    tex = gs_texture_create(size.cx, size.cy, GS_BGRA, 1, &data, GS_DYNAMIC);
    obs_leave_graphics();
    cx = (uint32_t)size.cx;  cy = (uint32_t)size.cy;
} else if (tex) {
    obs_enter_graphics();
    gs_texture_set_image(tex, bits.get(), size.cx * 4, false);
    obs_leave_graphics();
}
```
  (`obs_enter_graphics()` is required because `RenderText` is invoked from `update`/UI thread, not the graphics thread.) Draw side, `TextSource::Render()` lines 886-908 (called from `video_render`, graphics thread, no enter/leave needed):
```cpp
gs_effect_t *effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
gs_technique_t *tech = gs_effect_get_technique(effect, "Draw");
const bool previous = gs_framebuffer_srgb_enabled();
gs_enable_framebuffer_srgb(true);
gs_technique_begin(tech); gs_technique_begin_pass(tech, 0);
gs_effect_set_texture_srgb(gs_effect_get_param_by_name(effect, "image"), tex);
gs_draw_sprite(tex, 0, cx, cy);
gs_technique_end_pass(tech); gs_technique_end(tech);
gs_enable_framebuffer_srgb(previous);
```
- QImage/QPainter variant in the wild: **lulou/obs-qtwebengine** â€” `src/obsqtwebengine-renderer.cpp` line 98-110: `QImage image(&data->data, width, height, QImage::Format_RGBA8888); QPainter painter(&image); view.page()->view()->render(&painter);` and `src/obsqtwebengine-source.cpp`: `activeTexture = gs_texture_create(width, height, GS_RGBA, 1, nullptr, GS_DYNAMIC);` (line 41) + `obs_enter_graphics(); gs_texture_set_image(activeTexture, manager->GetData(), width * 4, false); obs_leave_graphics();` (lines 75-76). (Old project; pattern is valid. Note `QWidget::render` requires the GUI thread; pure `QPainter` onto a `QImage` is thread-safe in Qt 6 and can run on your event thread.) Double-buffering pattern to copy: paint into a back `QImage` on the worker thread, `std::swap` under a mutex, upload the front buffer in `video_tick`/`video_render` (graphics thread â€” no enter/leave needed there) or under `obs_enter_graphics()` from other threads, exactly as obs-text does. Match formats: `QImage::Format_ARGB32_Premultiplied` â†” `GS_BGRA` on little-endian, or `Format_RGBA8888` â†” `GS_RGBA`.

### 4.2 Pattern (b): composite child private sources
Canonical in-tree example â€” `plugins/image-source/obs-slideshow.c`:
- create: `source = obs_source_create_private("image_source", NULL, settings);` (line 167); `new_tr = obs_source_create_private(tr_name, NULL, NULL);` (line 329)
- activation ref so the child gets shown/active state: `obs_source_add_active_child(ss->source, new_tr);` (line 478)
- render inside the parent's `video_render`: `obs_source_video_render(transition);` (line 708)
- (release children with `obs_source_release`, implement `enum_active_sources`.)
Registered with `OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_COMPOSITE`-style flags in the same file. A third, simpler production pattern (no private child at all): **occ-ai/obs-urlsource** pushes text into an existing user-selected text source â€” `src/url-source-callbacks.cpp` lines 40-59: `obs_data_t *target_settings = obs_source_get_settings(target); obs_data_set_string(target_settings, "text", str.c_str()); obs_source_update(target, target_settings);`.

### 4.3 Windows text source id in OBS 31+ â€” **"text_gdiplus_v3": confirmed, with an important nuance**
`plugins/obs-text/gdiplus/obs-text.cpp` `obs_module_load` (lines 1110-1185): registers `si.id = "text_gdiplus"` (version unset), `si_v2.version = 2`, and:
```cpp
obs_source_info si_v3 = si_v2;
si_v3.version = 3;
si_v3.output_flags &= ~OBS_SOURCE_CAP_OBSOLETE;   // v3 is the only non-obsolete one
```
libobs then builds the *versioned* id at registration â€” `libobs/obs-module.c` lines 1042-1050:
```c
data.unversioned_id = data.id;
if (data.version) {
    struct dstr versioned_id = {0};
    dstr_printf(&versioned_id, "%s_v%d", data.id, (int)data.version);
    data.id = versioned_id.array;
}
```
and `obs_source_create_private(id, â€¦)` resolves by **exact match on the versioned id** (`libobs/obs-source.c` `get_source_info` lines 53-62, used at line 442). Therefore `obs_source_create_private("text_gdiplus_v3", NULL, settings)` is correct on Windows. Availability: **v3 exists since OBS 30.2.0** (present in tags 30.2.0/30.2.3/31.0.0/31.1.1/master; absent in 30.1.2). For forward-compat use `const char *id = obs_get_latest_input_type_id("text_gdiplus");` (`libobs/obs.c` line 1744) instead of hardcoding. Cross-platform ids: `text_ft2_source_v2` (freetype2, mac/linux).

---

## 5. Threading rules

### 5.1 obs_queue_task â€” exact API (libobs/obs.h lines 923-938)
```c
typedef void (*obs_task_t)(void *param);

enum obs_task_type {
    OBS_TASK_UI,
    OBS_TASK_GRAPHICS,
    OBS_TASK_AUDIO,
    OBS_TASK_DESTROY,
};

EXPORT void obs_queue_task(enum obs_task_type type, obs_task_t task, void *param, bool wait);
EXPORT bool obs_in_task_thread(enum obs_task_type type);
EXPORT bool obs_wait_for_destroy_queue(void);

typedef void (*obs_task_handler_t)(obs_task_t task, void *param, bool wait);
EXPORT void obs_set_ui_task_handler(obs_task_handler_t handler);
```
Semantics from the implementation (`libobs/obs.c:3309-3354`): `OBS_TASK_UI` â†’ forwarded to the registered `ui_task_handler` (error-logged if none); other types run inline if already on that thread; `wait=true` blocks on an `os_event` until executed; `OBS_TASK_GRAPHICS`/`OBS_TASK_AUDIO` push onto deques drained by the video/audio threads; `OBS_TASK_DESTROY` queues onto a dedicated `os_task_queue` destruction thread. OBS Studio installs the UI handler in `frontend/OBSApp.cpp:1216-1223, 1275`:
```cpp
static void ui_task_handler(obs_task_t task, void *param, bool wait)
{
    auto doTask = [=]() { task(param); };
    QMetaObject::invokeMethod(App(), "Exec", wait ? WaitConnection() : Qt::AutoConnection, Q_ARG(VoidFunc, doTask));
}
...
obs_set_ui_task_handler(ui_task_handler);
```
So **`obs_queue_task(OBS_TASK_UI, fn, param, false)` is the sanctioned way to hop any plugin thread onto the Qt main thread without linking Qt** â€” with `wait=true` it becomes a blocking call (`WaitConnection()` = `Qt::BlockingQueuedConnection` off-thread, `Qt::DirectConnection` on-thread; `shared/qt/wrappers/qt-wrappers.hpp:82-85`).

### 5.2 Which obs_frontend_* calls are safe from a WebSocket server thread (evidence: `frontend/OBSStudioAPI.cpp`)
- **Safe from any thread â€” internally marshalled**: `obs_frontend_recording_start/stop` (lines 239-247: `QMetaObject::invokeMethod(main, "StartRecording")`), `recording_pause` (256), `streaming_start/stop` (224-232), `replay_buffer_save/stop` (300-308: `"ReplayBufferSave"`), `take_screenshot` (582-590), `set_current_scene` (66-74, uses `WaitConnection()`), `add_scene_collection` (161-167, blocking wait + return value). Actives are lock-free atomics: `obs_frontend_recording_active()` = `os_atomic_load_bool(&recording_active)` (249-252).
- **Safe from any thread â€” no UI touched at all**: `obs_frontend_recording_add_chapter` (lines 279-291) does a direct `proc_handler_call(obs_output_get_proc_handler(fileOutput), "add_chapter", &cd)` with `calldata_set_string(&cd, "chapter_name", name)`.
- **UI-thread-only â€” they touch Qt widgets unmarshalled**: `obs_frontend_add_dock_by_id` (336-380, `new OBSDock(main)`), `obs_frontend_get_scenes` (43-54, iterates `main->ui->scenes` QListWidget items!), `obs_frontend_get_main_window`, menu/translation helpers. Call these from `obs_module_load`, frontend event callbacks, or via `obs_queue_task(OBS_TASK_UI, â€¦)` / `QMetaObject::invokeMethod`.
- This is precisely why obs-websocket can run `RequestHandler::StartRecord()` â†’ `obs_frontend_recording_start();` directly on its QThreadPool worker threads (`src/requesthandler/RequestHandler_Record.cpp` lines 66-99) â€” the marshalling lives inside the frontend API for those calls.
- **Frontend event callbacks run synchronously on the UI thread** (`OBSStudioAPI::on_event`, line 745, invoked from OBSBasic UI code paths) â€” safe place to create/remove docks; don't block in them.
- **Graphics rule**: all `gs_*` calls only on the graphics thread (`video_render`/`video_tick`/graphics tasks) or bracketed by `obs_enter_graphics()`/`obs_leave_graphics()` from other threads (see obs-text Â§4.1). libobs **signal handlers** (`obs_output` `"start"/"stop"/"saved"`, global `"source_*"`) fire on arbitrary libobs threads â†’ marshal to Qt exactly like vertical-canvas lines 5627-5658 (`QMetaObject::invokeMethod(obj, "Slot", Qt::QueuedConnection, Q_ARG(int, code), Q_ARG(QString, err))`; only meta-registered types in `Q_ARG`).
- Recommended architecture for your plugin: WS thread parses JSON â†’ for recording/replay/chapter actions call the frontend API directly (safe subset above); for dock/event-log UI updates `QMetaObject::invokeMethod(dockPtr, â€¦, Qt::QueuedConnection)`; for anything ambiguous `obs_queue_task(OBS_TASK_UI, â€¦)`. On unload: stop the asio server, `join()` the thread **before** returning from `obs_module_unload` (obs-websocket does `_server.stop_listening(); close all; _serverThread.join();` lines 140-161).

---

## 6. Config persistence

### 6.1 Exact APIs
- `libobs/obs-module.h:163`: `#define obs_module_config_path(file) obs_module_get_config_path(obs_current_module(), file)` â€” doc comment: "Returns the location to a module config file associated with the current module. Free with bfree when complete. Will return NULL if configuration directory is not set." Underlying decl `libobs/obs.h:661`: `EXPORT char *obs_module_get_config_path(obs_module_t *module, const char *file);`. (On Windows under OBS Studio this resolves to `%APPDATA%\obs-studio\plugin_config\<module-name>\<file>`.) **The directory is NOT auto-created.**
- `libobs/util/platform.h:157-162`:
```c
#define MKDIR_EXISTS 1
#define MKDIR_SUCCESS 0
#define MKDIR_ERROR -1
EXPORT int os_mkdir(const char *path);
EXPORT int os_mkdirs(const char *path);
```
- `libobs/obs-data.h:62-75`:
```c
EXPORT obs_data_t *obs_data_create_from_json_file(const char *json_file);
EXPORT obs_data_t *obs_data_create_from_json_file_safe(const char *json_file, const char *backup_ext);
EXPORT bool obs_data_save_json(obs_data_t *data, const char *file);
EXPORT bool obs_data_save_json_safe(obs_data_t *data, const char *file, const char *temp_ext, const char *backup_ext);
EXPORT bool obs_data_save_json_pretty_safe(obs_data_t *data, const char *file, const char *temp_ext, const char *backup_ext);
```

### 6.2 The pattern to copy verbatim â€” Aitum/obs-vertical-canvas `vertical-canvas.cpp:71-124` and 675-678
```cpp
static void ensure_directory(char *path)
{
#ifdef _WIN32
    char *backslash = strrchr(path, '\\');
    if (backslash) *backslash = '/';
#endif
    char *slash = strrchr(path, '/');
    if (slash) {
        *slash = 0;
        os_mkdirs(path);
        *slash = '/';
    }
#ifdef _WIN32
    if (backslash) *backslash = '\\';
#endif
}

static void save_canvas()
{
    char *path = obs_module_config_path("config.json");
    if (!path) return;
    ensure_directory(path);
    obs_data_t *config = obs_data_create_from_json_file_safe(path, "bak");
    if (!config) { config = obs_data_create(); }
    /* ... obs_data_set_array(config, "canvas", canvas) ... */
    if (obs_data_save_json_safe(config, path, "tmp", "bak")) { blog(LOG_INFO, "..."); }
    obs_data_release(config);
    bfree(path);
}
```
Load side (`obs_module_post_load`, line 675): `const auto path = obs_module_config_path("config.json"); obs_data_t *config = obs_data_create_from_json_file_safe(path, "bak");`. Save is triggered from the frontend event callback on `OBS_FRONTEND_EVENT_EXIT` / `OBS_FRONTEND_EVENT_SCRIPTING_SHUTDOWN` (line 136-138). `"tmp"`/`"bak"` extensions give atomic-write + rollback (`save_json_safe` writes `file.tmp`, then swaps, keeping `file.bak`).
Alternative (C++/nlohmann, obs-websocket `src/Config.cpp:197-209`): ensure dir with `std::filesystem::create_directories(std::filesystem::u8path(GetModuleConfigPath("")))`; its `Config::Load/Save` (lines 50-152) reads/writes `config.json` under the module config path and layers `--websocket_port/--websocket_password` command-line overrides on top â€” a good model if you also want CLI overrides for your event-port.

---

## Version-constraint cheat-sheet
| Thing | Constraint (verified) |
|---|---|
| CMake | â‰¥ 3.28 (presets + `cmake_minimum_required(VERSION 3.28...3.30)`); README recommends 3.30.5 on Windows |
| Windows toolchain | Visual Studio 17 2022 generator, arch `x64,version=10.0.22621`; CI runner `windows-2022` |
| Template pins (master, commits thru 2025-12-09) | obs-studio **31.1.1**, obs-deps prebuilt + qt6 **2025-07-11** (= Qt **6.8.3**) |
| Current OBS vs template | OBS **32.1.2** released 2026-04-21 (docs already at 32.2.0); bump `buildspec.json` versions+hashes to target 32.x |
| obs-deps extras usable for WS server (Win) | asio **1.32.0**, websocketpp **0.8.2**, nlohmann-json **3.11.3** (also at the 2025-07-11 tag) |
| Qt modules shipped with OBS | qtbase (incl. **Qt6Network**), qtimageformats, qtshadertools, qtmultimedia, qtsvg, qttools â€” **no Qt6WebSockets** |
| `obs_frontend_add_dock_by_id` | OBS â‰¥ 30.0.0 |
| `obs_frontend_recording_add_chapter` / `recording_split_file` | OBS â‰¥ 30.2.0 (chapters need Hybrid MP4 output) |
| `text_gdiplus_v3` | OBS â‰¥ 30.2.0 (Windows-only source; use `obs_get_latest_input_type_id("text_gdiplus")` to stay forward-compatible) |
| obs-websocket | 5.7.4 in-tree; RPC version 1; requires `ASIO_STANDALONE`, and on Windows `_WEBSOCKETPP_CPP11_STL_`, `_WIN32_WINNT=0x0603` |
