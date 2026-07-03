/*
GamePulse for OBS — event taxonomy.

Maps (game id, raw event name) -> display label + default importance, plus
game id -> display name. Unknown games/events degrade gracefully: the label
is a prettified event name and importance defaults to "minor", so any game
Overwolf GEP supports works out of the box even without a table entry here.
*/

#pragma once

#include <string>

#include "gp-types.h"

namespace gamepulse {

struct EventMeta {
	std::string label;
	int importance = IMP_MINOR;
	bool known = false;
};

class Taxonomy {
public:
	/* Display name for an Overwolf game class id ("21640" -> "VALORANT").
	   Returns fallback (usually the companion-provided name) when unknown. */
	static std::string game_name(const std::string &game_id, const std::string &fallback);

	/* Label + importance for a raw or derived event. */
	static EventMeta lookup(const std::string &game_id, const std::string &event_name);

	/* "spike_planted" -> "Spike Planted" */
	static std::string prettify(const std::string &event_name);
};

} // namespace gamepulse
