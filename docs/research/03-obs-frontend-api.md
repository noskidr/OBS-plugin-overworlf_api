# OBS Studio Native Plugin API Verification (Windows, OBS 30.2 â†’ mid-2026)

All signatures below were pulled from the **actual headers/sources** at `raw.githubusercontent.com/obsproject/obs-studio` on tags `30.2.0`, `31.0.0`, `31.1.0`, `32.0.0`, `32.1.2` and `master`, plus the docs sources (`docs/sphinx/*.rst`) which carry official `.. versionadded::` annotations.

## 0. Version landscape (verified via GitHub Releases API, July 3 2026)

| Release | Date | Frontend header path | Qt shipped (Windows, from obs-deps tag) |
|---|---|---|---|
| 30.2.0 | 2024-07-12 | `UI/obs-frontend-api/obs-frontend-api.h` | Qt **6.6.3** (deps 2024-05-08) |
| 31.0.0 | 2024-12-07 | `UI/obs-frontend-api/obs-frontend-api.h` | Qt **6.6.3** (deps 2024-09-12) |
| 31.1.0 | 2025-07-07 | **moved â†’** `frontend/api/obs-frontend-api.h` | Qt **6.8.3** (deps 2025-05-23) |
| 32.0.0 | 2025-09-22 | `frontend/api/obs-frontend-api.h` | Qt **6.8.3** (deps 2025-08-23) |
| **32.1.2 = current stable** | 2026-04-21 | `frontend/api/obs-frontend-api.h` | Qt **6.8.3** (deps 2025-08-23) |
| 32.2.0-beta3 (prerelease) | 2026-07-03 | `frontend/api/obs-frontend-api.h` | Qt **6.11.1** (deps 2026-06-25, master) |

- **Current stable as of July 2026: OBS Studio 32.1.2** (`tag_name: "32.1.2"`, published 2026-04-21). 32.2 is in beta (beta3 published 2026-07-03). `master` identifies as 32.2.0-dev: `libobs/obs-config.h` â†’ `#define LIBOBS_API_MAJOR_VER 32 / LIBOBS_API_MINOR_VER 2 / LIBOBS_API_PATCH_VER 0`, and docs.obsproject.com is titled "OBS Studio 32.2.0".
- **Header move**: `UI/obs-frontend-api/obs-frontend-api.h` exists through 31.0.x; from **31.1.0** the file is `frontend/api/obs-frontend-api.h` (the old path 404s on master; verified). The installed include name is unchanged: `#include <obs-frontend-api.h>`.
- **No `OBS_FRONTEND_API_VERSION` macro exists.** The frontend API is versioned only by OBS release; docs mark functions with `.. versionadded::`. Gate at compile time with `LIBOBS_API_VER` (`libobs/obs-config.h`), at runtime with `obs_get_version()` / `obs_get_version_string()`. On Windows, referencing a symbol absent from the user's `obs-frontend-api.dll` makes the plugin DLL fail to load â€” every function below already exists in 30.2.0, so a 30.2+ plugin can link them all directly.
- API churn relevant to 30.2â†’32.2 (from header diffs): 31.0 deprecated `obs_frontend_get_global_config()` and added `obs_frontend_get_app_config()` / `obs_frontend_get_user_config()` (versionadded 31.0); 31.1 added the canvas API + `OBS_FRONTEND_EVENT_CANVAS_ADDED/REMOVED`; **32.0 removed** the long-deprecated `void *obs_frontend_add_dock(void *dock)`; 32.2 (master) adds `obs_frontend_copy_sceneitem` / `obs_frontend_can_paste_sceneitem` / `obs_frontend_paste_sceneitem` (versionadded 32.2).

## 1. `obs_frontend_recording_add_chapter`

Header (identical 30.2.0 â†’ master):
```c
EXPORT bool obs_frontend_recording_add_chapter(const char *name);
```
- **Min OBS version: 30.2** â€” docs `reference-frontend-api.rst`: ".. versionadded:: 30.2". Present in every release 30.2.0 â†’ 32.1.2 â†’ master (verified in each header).
- Official docs text: "Asks OBS to insert a chapter marker at the current output time into the recording." `name` "may be NULL to use an automatically generated name ('Unnamed <Chapter number>' or localized equivalent)". **Return:** "*true* if insertion was successful, *false* if recording is inactive, paused, or if chapter insertion is not supported by the current output."
- Implementation (master `frontend/OBSStudioAPI.cpp`, 30.2 `UI/api-interface.cpp` â€” identical logic): returns `false` if `!recording_active || recording_paused`; otherwise sets calldata string `"chapter_name"` and returns `proc_handler_call(ph, "add_chapter", &cd)` on the recording `fileOutput`'s proc handler. `proc_handler_call` returns false when the output has no `add_chapter` proc â†’ that is the "unsupported container" path.
- **Container constraint: Hybrid MP4 only (plus Hybrid MOV from 32.0).** The proc `"void add_chapter(string chapter_name)"` is registered **only** in `plugins/obs-outputs/mp4-output.c` (`mp4_output_create_internal`), i.e. output id `"mp4_output"` (Hybrid MP4; 30.2+) and `"mov_output"` (Hybrid MOV; added 32.0 â€” absent in 30.2.0 and 31.1.0 sources). The generic FFmpeg muxer `plugins/obs-ffmpeg/obs-ffmpeg-mux.c` (`ffmpeg_muxer` â€” used for **mkv**, fragmented MP4/MOV, plain MP4, TS) registers **no** `add_chapter` proc in 30.2.0, 32.1.2, or master â†’ **MKV recordings cannot receive chapter markers, in any OBS version to date.** Your plugin should treat a `false` return as "container doesn't support chapters" (after checking active/paused itself), or pre-check `obs_output_get_id(obs_frontend_get_recording_output()) == "mp4_output"/"mov_output"`.
- Placement semantics (mp4-output.c): the proc stamps the chapter with `obs_get_video_frame_time()` at call time, queues it, and writes it when the video packet whose CTS reaches that timestamp passes through the muxer; chapter DTS = `pkt->pts*1000000/timebase_den - start_time` (i.e., **media time within the file**). Empty name â†’ `"MP4Output.UnnamedChapter" + counter`. Log line: `Adding chapter "%s" at %02d:%02d:%02d.%03d`.
- OBS also exposes a built-in hotkey `"OBSBasic.AddChapterMarker"` (frontend/widgets/OBSBasic_Hotkeys.cpp).

## 2. Replay buffer

Header (all present in 30.2.0 â†’ master):
```c
EXPORT void obs_frontend_replay_buffer_start(void);
EXPORT void obs_frontend_replay_buffer_save(void);
EXPORT void obs_frontend_replay_buffer_stop(void);
EXPORT bool obs_frontend_replay_buffer_active(void);
EXPORT obs_output_t *obs_frontend_get_replay_buffer_output(void); /* "A new reference ... Release with obs_output_release()" */
EXPORT char *obs_frontend_get_last_replay(void);                  /* versionadded 29.0.0; "Free with bfree()" */
```
- Replay buffer output: id **`"replay_buffer"`**, `plugins/obs-ffmpeg/obs-ffmpeg-mux.c`, flags `OBS_OUTPUT_AV | OBS_OUTPUT_ENCODED | OBS_OUTPUT_MULTI_TRACK | OBS_OUTPUT_CAN_PAUSE`.
- Procs/signals registered in `replay_buffer_create` (master lines 961â€“965; same in 30.2.0):
```c
proc_handler_add(ph, "void save()", save_replay_proc, stream);
proc_handler_add(ph, "void get_last_replay(out string path)", get_last_replay, stream);
signal_handler_add(sh, "void saved()");
```
- **Save flow (async):** `obs_frontend_replay_buffer_save()` â†’ `OBSBasic::ReplayBufferSave` â†’ proc `"save"`. When the muxer finishes writing, it fires `signal_handler_signal(sh, "saved", &cd)`. The frontend connects this in `frontend/utility/SimpleOutput.cpp:246` / `AdvancedOutput.cpp:110`: `replayBufferSaved.Connect(signal, "saved", OBSReplayBufferSaved, this)` â†’ queued to the UI thread â†’ `OBSBasic::ReplayBufferSaved()` calls proc `"get_last_replay"` (reads calldata string `"path"`), sets `main->lastReplay`, **then** fires `OBS_FRONTEND_EVENT_REPLAY_BUFFER_SAVED`. Consequence: calling `obs_frontend_get_last_replay()` inside your `REPLAY_BUFFER_SAVED` event callback reliably returns the just-saved file path.
- A plugin may also connect directly: `signal_handler_connect(obs_output_get_signal_handler(replay), "saved", cb, ud)` and/or call the `"get_last_replay"` proc itself (`calldata_string(&cd, "path")`).

## 3. Recording state & elapsed time

Header:
```c
EXPORT void obs_frontend_recording_start(void);
EXPORT void obs_frontend_recording_stop(void);
EXPORT bool obs_frontend_recording_active(void);
EXPORT void obs_frontend_recording_pause(bool pause);
EXPORT bool obs_frontend_recording_paused(void);
EXPORT bool obs_frontend_recording_split_file(void);   /* false if inactive/paused/splitting disabled */
EXPORT obs_output_t *obs_frontend_get_recording_output(void); /* new reference */
EXPORT char *obs_frontend_get_last_recording(void);    /* versionadded 29.0.0; bfree() */
EXPORT char *obs_frontend_get_current_record_output_path(void);
```
- `obs_frontend_get_last_recording()` returns `outputHandler->lastRecordingPath`, which is set **when the filename is generated at recording start** (`BasicOutputHandler::GetRecordingFilename`, frontend/utility/BasicOutputHandler.cpp:449) and updated on every split via the `"file_changed"` handler (line 112). I.e. during recording it is the *current* file, not only available after stop.
- **Robust elapsed time â€” frames-based (what obs-websocket ships as `outputDuration`):** `plugins/obs-websocket` â†’ repo obsproject/obs-websocket, `src/utils/Obs_NumberHelper.cpp`:
```cpp
uint64_t Utils::Obs::NumberHelper::GetOutputDuration(obs_output_t *output)
{
	if (!output || !obs_output_active(output))
		return 0;
	video_t *video = obs_output_video(output);
	uint64_t frameTimeNs = video_output_get_frame_time(video);
	int totalFrames = obs_output_get_total_frames(output);
	return util_mul_div64(totalFrames, frameTimeNs, 1000000ULL); /* milliseconds */
}
```
Underlying signatures: `EXPORT int obs_output_get_total_frames(const obs_output_t *output);` (libobs/obs.h:2073), `EXPORT uint64_t video_output_get_frame_time(const video_t *video);` and `EXPORT double video_output_get_frame_rate(const video_t *video);` (libobs/media-io/video-io.h:316/323).
- **Pause handling:** the recording outputs are `OBS_OUTPUT_CAN_PAUSE`; while paused no frames reach the output, so `obs_output_get_total_frames` freezes â†’ the frames-based value equals **in-file media time (pauses excluded)** â€” exactly the same timeline the Hybrid MP4 muxer uses for chapter DTS. This makes frames-based time the correct one for stamping game events into recordings. Extra helpers: `EXPORT bool obs_output_paused(const obs_output_t *output);` / `EXPORT bool obs_output_pause(obs_output_t *output, bool pause);` (obs.h:1975â€“1978) and `EXPORT uint64_t obs_output_get_pause_offset(obs_output_t *output);` (obs.h:2216; cumulative pause offset in ns â€” docs: "Returns the current pause offset of the output. Used with raw outputs to calculate system timestamps when using calculated timestamps").
- OBS's own status bar does NOT use frames: it increments `totalRecordSeconds` on a 1-second QTimer and skips ticks while `os_atomic_load_bool(&recording_paused)` (frontend/widgets/OBSBasicStatusBar.cpp:278â€“312) â€” a wall-clock approximation that can drift. Prefer the frames method.
- **Split caveat:** automatic/manual file splits do NOT restart the output, so `total_frames` keeps counting across files. For "offset within the current file", record the media time at each `"file_changed(string next_file)"` signal (registered by both `ffmpeg_muxer` â€” obs-ffmpeg-mux.c:104 â€” and the mp4 muxer â€” mp4-output.c) and subtract.

## 4. Streaming state & elapsed time

```c
EXPORT void obs_frontend_streaming_start(void);
EXPORT void obs_frontend_streaming_stop(void);
EXPORT bool obs_frontend_streaming_active(void);
EXPORT obs_output_t *obs_frontend_get_streaming_output(void); /* docs: "A new reference to the current streaming output. Release with obs_output_release()." */
```
- "Seconds since stream start": same frames-based `GetOutputDuration` on the streaming output (this is obs-websocket `GetStreamStatus.outputDuration`). It counts encoded output frames, so it represents time the output has been running and is immune to UI-thread stalls; wall-clock from `OBS_FRONTEND_EVENT_STREAMING_STARTED` also works and is what OBS's status bar effectively does (`totalStreamSeconds++` on a 1 s timer, OBSBasicStatusBar.cpp:240). For Twitch-marker-style offsets, capture `os_gettime_ns()` (or `std::chrono::steady_clock`) at `STREAMING_STARTED` and also sample `GetOutputDuration` as a cross-check; note reconnects keep the output active (see `obs_output_reconnecting()`, obs.h:2133). `obs_output_get_connect_time_ms(obs_output_t*)` (obs.h:2131) is time-to-connect, not elapsed time.

## 5. Frontend event enum (master `frontend/api/obs-frontend-api.h` lines 16â€“68)

All of the following already exist in **30.2.0** with the same relative order; the enum is append-only from 30.2 â†’ 32.2, so numeric values of pre-existing entries are unchanged (`OBS_FRONTEND_EVENT_STREAMING_STARTING`=0 â€¦ `OBS_FRONTEND_EVENT_EXIT`=17, `REPLAY_BUFFER_STARTING/STARTED/STOPPING/STOPPED`=18â€“21, `FINISHED_LOADING`=26, `RECORDING_PAUSED/UNPAUSED`=27/28, `REPLAY_BUFFER_SAVED`=30, `SCRIPTING_SHUTDOWN`=38, `THEME_CHANGED`=41, `SCREENSHOT_TAKEN`=42):
```c
enum obs_frontend_event {
	OBS_FRONTEND_EVENT_STREAMING_STARTING, OBS_FRONTEND_EVENT_STREAMING_STARTED,
	OBS_FRONTEND_EVENT_STREAMING_STOPPING, OBS_FRONTEND_EVENT_STREAMING_STOPPED,
	OBS_FRONTEND_EVENT_RECORDING_STARTING, OBS_FRONTEND_EVENT_RECORDING_STARTED,
	OBS_FRONTEND_EVENT_RECORDING_STOPPING, OBS_FRONTEND_EVENT_RECORDING_STOPPED,
	/* ... scene/transition/collection/profile events ..., OBS_FRONTEND_EVENT_EXIT, */
	OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING, OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED,
	OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING, OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED,
	/* studio mode, preview scene */
	OBS_FRONTEND_EVENT_SCENE_COLLECTION_CLEANUP, OBS_FRONTEND_EVENT_FINISHED_LOADING,
	OBS_FRONTEND_EVENT_RECORDING_PAUSED, OBS_FRONTEND_EVENT_RECORDING_UNPAUSED,
	OBS_FRONTEND_EVENT_TRANSITION_DURATION_CHANGED, OBS_FRONTEND_EVENT_REPLAY_BUFFER_SAVED,
	OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED, OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED,
	OBS_FRONTEND_EVENT_TBAR_VALUE_CHANGED, OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGING,
	OBS_FRONTEND_EVENT_PROFILE_CHANGING, OBS_FRONTEND_EVENT_SCRIPTING_SHUTDOWN,
	OBS_FRONTEND_EVENT_PROFILE_RENAMED, OBS_FRONTEND_EVENT_SCENE_COLLECTION_RENAMED,
	OBS_FRONTEND_EVENT_THEME_CHANGED, OBS_FRONTEND_EVENT_SCREENSHOT_TAKEN,
	OBS_FRONTEND_EVENT_CANVAS_ADDED, OBS_FRONTEND_EVENT_CANVAS_REMOVED, /* added 31.1.0 */
};
```
- Docs semantics: `SCRIPTING_SHUTDOWN` â€” "Triggered when scripts need to know that OBS is exiting. The OBS_FRONTEND_EVENT_EXIT event is normally called after scripts have been destroyed." `EXIT` â€” "the last chance to call any frontend API functions for any saving / cleanup"; after returning you must not call frontend API. `RECORDING_STOPPING` fires before `RECORDING_STOPPED`; `REPLAY_BUFFER_SAVED` fires after `lastReplay` is updated (see Â§2).
- Callback registration:
```c
typedef void (*obs_frontend_event_cb)(enum obs_frontend_event event, void *private_data);
EXPORT void obs_frontend_add_event_callback(obs_frontend_event_cb callback, void *private_data);
EXPORT void obs_frontend_remove_event_callback(obs_frontend_event_cb callback, void *private_data);
```
- Threading: events are dispatched synchronously from `OBSBasic::OnEvent(...)` on the **Qt UI thread** (e.g., `ReplayBufferSaved` runs on the main thread via `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`). Don't block in the callback; marshal to your own thread (e.g., the WebSocket server thread) for network I/O.

## 6. Docks

Master header (comment included verbatim):
```c
/* takes QWidget for widget */
EXPORT bool obs_frontend_add_dock_by_id(const char *id, const char *title, void *widget);
EXPORT void obs_frontend_remove_dock(const char *id);
/* takes QDockWidget for dock */
EXPORT bool obs_frontend_add_custom_qdock(const char *id, void *dock);
```
- All three: **versionadded 30.0** (docs) â†’ safe for the whole 30.2+ window. Return: "*true* if the dock was added, *false* if the id was already used".
- `add_dock_by_id` docs details: dock gets a toggle in the Docks menu; on dock close, a custom `QEvent` of type `QEvent::User + QEvent::Close` is sent to the widget "to enable it to react (e.g., unload elements to save resources)"; a normal `QShowEvent` arrives when shown. Remove with `obs_frontend_remove_dock(id)`.
- The old `OBS_DEPRECATED EXPORT void *obs_frontend_add_dock(void *dock);` still exists in 30.2.0â€“31.1.x but was **deleted in 32.0.0** (verified: absent from the 32.0.0 header). Do not use it if you support 32.x.
- Qt requirements: `widget` is a `QWidget*` passed as `void*`; your plugin must build against **Qt 6 Widgets** (`find_package(Qt6 COMPONENTS Widgets)`; obs-plugintemplate's `ENABLE_QT` option). OBS ships Qt 6.6.3 (30.2/31.0), 6.8.3 (31.1/32.0/32.1), 6.11.1 (32.2 beta/master) on Windows â€” build against the matching obs-deps Qt for binary compatibility. Create the widget and call `obs_frontend_add_dock_by_id` on the UI thread (`obs_module_load` and frontend event callbacks run there). Keep the dock `id` stable across sessions â€” OBS persists dock layout/visibility by id in the window state.

## 7. Hotkeys

`libobs/obs-hotkey.h` (master; unchanged in this window):
```c
typedef size_t obs_hotkey_id;
#define OBS_INVALID_HOTKEY_ID (~(obs_hotkey_id)0)
typedef void (*obs_hotkey_func)(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed);

EXPORT obs_hotkey_id obs_hotkey_register_frontend(const char *name, const char *description, obs_hotkey_func func,
						  void *data);
EXPORT obs_hotkey_pair_id obs_hotkey_pair_register_frontend(const char *name0, const char *description0,
							    const char *name1, const char *description1,
							    obs_hotkey_active_func func0, obs_hotkey_active_func func1,
							    void *data0, void *data1);
EXPORT void obs_hotkey_unregister(obs_hotkey_id id);
EXPORT void obs_hotkey_load(obs_hotkey_id id, obs_data_array_t *data);
EXPORT obs_data_array_t *obs_hotkey_save(obs_hotkey_id id);
EXPORT void obs_hotkey_load_bindings(obs_hotkey_id id, obs_key_combination_t *combinations, size_t num);
```
- **Persistence (verified in source, and this is the part most guides get wrong):** the Settingsâ†’Hotkeys dialog saves **every hotkey whose registerer type is `OBS_HOTKEY_REGISTERER_FRONTEND` â€” including third-party plugins'** â€” into the **profile's `basic.ini` `[Hotkeys]` section**, keyed by the hotkey *name*, value = JSON `{"bindings":[...]}` from `obs_hotkey_save` (`frontend/settings/OBSBasicSettings.cpp` `SaveHotkeySettings()`, lines 3723â€“3756; the include-filter is `hotkeys.emplace_back(registerer_type == OBS_HOTKEY_REGISTERER_FRONTEND, hw)` at line 2927). **However, on startup the frontend re-loads bindings only for its own built-ins** (`OBSBasic.*` names via `LoadHotkey`/`LoadHotkeyPair` in `frontend/widgets/OBSBasic_Hotkeys.cpp:94â€“127`) and the `"ReplayBuffer"` output hotkey (`SimpleOutput.cpp:227`, `AdvancedOutput.cpp:91`). An exhaustive grep of `frontend/` shows no generic loader. â†’ **Your plugin must restore its own bindings**, e.g. at `OBS_FRONTEND_EVENT_FINISHED_LOADING` / after registering:
```c
config_t *cfg = obs_frontend_get_profile_config();
const char *json = config_get_string(cfg, "Hotkeys", "MyPlugin.MarkClip"); /* written by OBS's settings dialog */
if (json) {
    obs_data_t *d = obs_data_create_from_json(json);
    obs_data_array_t *bindings = obs_data_get_array(d, "bindings");
    obs_hotkey_load(hotkey_id, bindings);
    obs_data_array_release(bindings); obs_data_release(d);
}
```
(Alternative used by many plugins: persist `obs_hotkey_save()` output yourself via `obs_frontend_add_save_callback` into scene-collection data.) Reload on `OBS_FRONTEND_EVENT_PROFILE_CHANGED` since `[Hotkeys]` lives in the profile. Hotkey `name` must be globally unique; `description` is the label shown in Settings â†’ Hotkeys.

## 8. Captions (`obs_output_output_caption_text2` â€” there is **no** `text3`)

`libobs/obs.h` (master lines 2125â€“2128; identical in 30.2.0):
```c
EXPORT void obs_output_caption(obs_output_t *output, const struct obs_source_cea_708 *captions);
EXPORT void obs_output_output_caption_text1(obs_output_t *output, const char *text);
EXPORT void obs_output_output_caption_text2(obs_output_t *output, const char *text, double display_duration);
```
- Docs (`reference-outputs.rst:639`): "Outputs captions from the specified text input. *text1* is the same as *text2*, except that the *display_duration* is hardcoded to 2.0 seconds. *display_duration* represents the minimum quantity of time that a given caption can be displayed for before moving onto the next caption in the queue."
- Injects **CEA-708** closed captions into the encoded bitstream (H.264 SEI; AV1; HEVC when `ENABLE_HEVC`) â€” codec branch identical in 30.2.0 and master (`libobs/obs-output.c` `add_caption`). Works on the live **streaming** output; Twitch renders these as CC.
- Canonical usage = obs-websocket 5.x `SendStreamCaption` (`src/requesthandler/RequestHandler_Stream.cpp:144`): checks `obs_frontend_streaming_active()`, then `OBSOutputAutoRelease output = obs_frontend_get_streaming_output(); obs_output_output_caption_text2(output, captionText.c_str(), 0.0); /* 0.0 = no delay until the next caption can be sent */`.

## 9. Tools menu

```c
typedef void (*obs_frontend_cb)(void *private_data);
EXPORT void *obs_frontend_add_tools_menu_qaction(const char *name); /* docs: "Adds a QAction to the tools menu then returns it" (returns QAction* as void*) */
EXPORT void obs_frontend_add_tools_menu_item(const char *name, obs_frontend_cb callback, void *private_data); /* docs: "Adds a tools menu item and links the ::clicked signal to the callback" */
```
Both long predate 30.2 (no versionadded) and are unchanged through master. `add_tools_menu_item` needs no Qt in your code; `..._qaction` requires Qt to use the returned `QAction*`.

## 10. Remaining cross-cutting answers

- **`obs_frontend_recording_add_chapter` exists in all of 30.2.0 â†’ 32.1.2 â†’ master.** Yes â€” verified in each tag's header.
- **Windows default recording container:**
  - 30.2.0 through 31.1.x **stable** builds: `#ifdef __APPLE__ "fragmented_mov" #elif OBS_RELEASE_CANDIDATE == 0 && OBS_BETA == 0 "mkv" #else "hybrid_mp4" #endif` â†’ **Windows default = `mkv`** (hybrid_mp4 only in beta/RC builds). (`UI/window-basic-main.cpp` at 30.2.0/31.0.0; `frontend/widgets/OBSBasic.cpp` at 31.1.0; applied via `config_set_default_string(activeConfiguration, "SimpleOutput"/"AdvOut", "RecFormat2", DEFAULT_CONTAINER)`.)
  - **32.0.0 and later: Windows/Linux default = `hybrid_mp4`, macOS = `hybrid_mov`, unconditionally** (`frontend/widgets/OBSBasic.cpp`, 32.0.0 lines 574â€“576; master lines 601â€“605). Matches the 32.0 release notes: "Hybrid MP4/MOV, now out of beta, has also been made the default output container for new profiles."
  - Defaults only apply to **new profiles** â€” existing users keep whatever `RecFormat2` they had (often mkv). So in the field you must expect both: chapter markers silently unsupported on mkv (call returns false); consider surfacing a hint to switch the recording format to Hybrid MP4, or fall back to writing your own sidecar marker file/export.
- **Qt major**: OBS ships **Qt 6 only** across the window (6.6.3 â†’ 6.8.3 â†’ 6.11.1 as tabled above). No Qt5 anywhere.
- **Frontend API implementation topology (useful when reading source):** `frontend/api/obs-frontend-api.cpp` is a thin dispatcher through a `obs_frontend_callbacks` vtable (`frontend/api/obs-frontend-internal.hpp`); the real logic is `frontend/OBSStudioAPI.cpp` (master) / `UI/api-interface.cpp` (â‰¤31.0.x).

## Practical implications for the planned plugin

1. Chapter markers: call `obs_frontend_recording_add_chapter(name)`; expect `false` on mkv/paused/inactive. Requires OBS â‰¥ 30.2 (your stated floor). Detect container via `obs_output_get_id()` of the recording output (`"mp4_output"`/`"mov_output"` = chapters OK).
2. Auto clips: drive `obs_frontend_replay_buffer_save()` and consume `OBS_FRONTEND_EVENT_REPLAY_BUFFER_SAVED` + `obs_frontend_get_last_replay()` (path is guaranteed fresh inside that callback). Free returned strings with `bfree()`.
3. Eventâ†’media-time mapping: on each game event, if `obs_frontend_recording_active()`, compute media-time ms via total-frames Ã— frame-time (obs-websocket formula) on `obs_frontend_get_recording_output()` (release the ref!). This aligns exactly with Hybrid-MP4 chapter timestamps and excludes pauses. Track `file_changed` for splits.
4. Stream markers: compute stream elapsed with the same formula on the streaming output, or wall clock anchored at `OBS_FRONTEND_EVENT_STREAMING_STARTED`; OBS has no native Twitch-marker API (do it via Helix from the companion app or the plugin).
5. Dock: `obs_frontend_add_dock_by_id("insights-event-log", obs_module_text("EventLog"), (void*)myQWidget)`; never touch the removed `obs_frontend_add_dock`. Build with Qt6 Widgets matching OBS's Qt (6.8.3 for current stable 32.1.2).
6. Hotkeys: `obs_hotkey_register_frontend("InsightsCapture.MarkMoment", obs_module_text("MarkMoment"), cb, ud)` + self-restore bindings from profile config `[Hotkeys]` as shown in Â§7.
7. Captions: `obs_output_output_caption_text2(streaming_output, text, 0.0)` for on-stream CC event tickers (H.264/HEVC/AV1).
8. Version gating: compile against current obs-studio headers; everything used is present since 30.2.0, so no dynamic symbol lookup needed; optionally warn at runtime via `obs_get_version_string()`.
