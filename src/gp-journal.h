/*
GamePulse for OBS — session journal + exporters.

Every notable event is appended to an in-memory session journal and mirrored
to a JSON file. On demand (or automatically when the stream/recording ends)
the journal renders:
  - YouTube chapter list (paste into the video description)
  - CSV (spreadsheet / Premiere-friendly)
  - EDL markers (DaVinci Resolve "File > Import Timeline > Pre-conformed EDL")
  - the raw session JSON

All calls must happen on the OBS main thread (the action executor lives there).
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "gp-types.h"

namespace gamepulse {

struct JournalEntry {
	int64_t wall_ms = 0;
	int64_t stream_ms = -1;
	int64_t record_ms = -1;
	std::string game_name;
	std::string name;
	std::string label;
	std::string detail;
	int importance = IMP_MINOR;
	uint32_t actions = ACTION_NONE;
	std::string source; /* "gep" | "manual" | "chat" */
};

class Journal {
public:
	/* base_dir: where session files live (created if missing). */
	void set_base_dir(const std::string &base_dir);
	const std::string &base_dir() const { return base_dir_; }

	/* Begin a new session (idempotent if already open). Session id is derived
	   from local wall time: YYYYMMDD-HHMMSS. */
	void begin_session(const char *reason);
	bool session_open() const { return session_open_; }
	std::string session_id() const { return session_id_; }

	void append(const GpEvent &ev);

	/* Number of entries so far. */
	size_t size() const { return entries_.size(); }
	const std::vector<JournalEntry> &entries() const { return entries_; }

	/* Write export files next to the session JSON. Returns the directory used.
	   which: bitmask — 1 json (always rewritten), 2 youtube, 4 csv, 8 edl. */
	std::string export_files(unsigned which = 0xF);

	/* Close the current session (writes final exports if auto_export). */
	void end_session(bool auto_export);

	/* Human summary line for the dock, e.g. "23 events - 4 clips - 6 chapters". */
	std::string summary() const;

private:
	std::string json_path() const;
	void write_json() const;
	std::string render_youtube() const;
	std::string render_csv() const;
	std::string render_edl() const;

	std::string base_dir_;
	std::string session_id_;
	std::string session_dir_;
	bool session_open_ = false;
	int64_t session_started_wall_ms_ = 0;
	std::vector<JournalEntry> entries_;
};

} // namespace gamepulse
