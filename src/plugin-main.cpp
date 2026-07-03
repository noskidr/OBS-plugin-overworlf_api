/*
GamePulse for OBS — plugin entry point.

Bridges live game events (delivered over a localhost WebSocket by the
Overwolf companion app) into OBS actions: recording chapter markers,
replay-buffer auto-clips, Twitch stream markers, an on-stream overlay
source, a control/event-log dock, hotkeys and post-session exports.
*/

#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QMainWindow>
#include <QAction>

#include "plugin-support.h"
#include "gp-core.h"
#include "gp-dock.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

extern "C" void gp_register_overlay_source(void);

using namespace gamepulse;

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "GamePulse loading (version %s)", PLUGIN_VERSION);

	gp_register_overlay_source();
	GpCore::instance().startup();

	return true;
}

void obs_module_post_load(void)
{
	/* Dock + hotkeys need the frontend/Qt up. */
	GpCore::instance().post_load();

	auto *dock = new GpDock();
	dock->setObjectName("GamePulseDock");
	obs_frontend_add_dock_by_id("GamePulseDock", obs_module_text("Dock.Title"), dock);

	obs_log(LOG_INFO, "GamePulse dock + overlay registered");
}

void obs_module_unload(void)
{
	GpCore::instance().shutdown();
	obs_log(LOG_INFO, "GamePulse unloaded");
}
