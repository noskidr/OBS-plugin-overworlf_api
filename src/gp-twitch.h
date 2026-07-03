/*
GamePulse for OBS — Twitch integration.

- OAuth Device Code Grant for a PUBLIC client (client_id only, no secret):
  POST id.twitch.tv/oauth2/device  -> user_code + verification_uri
  poll id.twitch.tv/oauth2/token   -> access/refresh tokens (rotating refresh)
- Stream markers: POST api.twitch.tv/helix/streams/markers
  (scope channel:manage:broadcast; only works while live with VODs on)
- Viewer chat commands: anonymous read-only IRC (justinfan) on
  irc.chat.twitch.tv:6667 with tags — no credentials needed, one channel.

All HTTP runs on worker threads (libcurl); callbacks fire on those worker
threads — marshal to the UI thread yourself (GpCore does).
*/

#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <obs-data.h>

namespace gamepulse {

class TwitchService {
public:
	TwitchService();
	~TwitchService();

	void load(obs_data_t *root);       /* reads "twitch" object */
	void save(obs_data_t *root) const; /* writes "twitch" object */

	void start_workers();
	void shutdown();

	bool authed() const;
	std::string login() const;
	std::string client_id() const;
	void set_client_id(const std::string &id);

	/* Device flow. on_code fires when the user code is ready (show it +
	   open the URL); on_done fires on success/failure/timeout. Worker
	   thread context! */
	void begin_device_auth(std::function<void(std::string user_code, std::string verification_uri)> on_code,
			       std::function<void(bool ok, std::string message)> on_done);
	void cancel_device_auth();
	void logout();

	/* Fire-and-forget; worker sends when possible. */
	void queue_marker(const std::string &description);

	/* Chat listener */
	struct ChatConfig {
		bool enabled = false;
		std::string command = "!clip";
		std::string permission = "mod"; /* anyone | sub | vip | mod | broadcaster */
		int cooldown_s = 60;
		std::string channel_override; /* empty -> authed login */
	};
	void set_chat_config(const ChatConfig &cfg);
	ChatConfig chat_config() const;
	void apply_chat_state(); /* start/stop IRC thread to match config */
	bool chat_running() const;

	/* callbacks into the host (worker/IRC thread context!) */
	std::function<void(std::string user, std::string channel)> on_chat_clip;
	std::function<void()> on_state_changed; /* tokens/login changed — persist soon */

private:
	struct Http {
		long code = 0;
		std::string body;
	};
	Http http_post_form(const std::string &url, const std::string &form_body,
			    const std::vector<std::string> &headers = {});
	Http http_post_json(const std::string &url, const std::string &json_body,
			    const std::vector<std::string> &headers);
	Http http_get(const std::string &url, const std::vector<std::string> &headers);

	bool ensure_fresh_token(); /* refresh if close to expiry; worker thread */
	bool refresh_tokens_locked(std::string refresh_token);
	bool fetch_identity(); /* GET /helix/users -> user_id + login */

	void marker_worker();
	void irc_worker();

	/* token state */
	mutable std::mutex state_mutex_;
	std::string client_id_;
	std::string access_token_;
	std::string refresh_token_;
	std::string user_id_;
	std::string login_;
	int64_t expires_at_ms_ = 0; /* monotonic deadline */

	/* marker queue */
	std::thread marker_thread_;
	std::mutex queue_mutex_;
	std::condition_variable queue_cv_;
	std::deque<std::string> marker_queue_;
	std::atomic<bool> workers_running_{false};

	/* device flow */
	std::thread device_thread_;
	std::atomic<bool> device_cancel_{false};
	std::atomic<bool> device_active_{false};

	/* chat */
	mutable std::mutex chat_mutex_; /* guards chat_cfg_ only */
	ChatConfig chat_cfg_;
	/* Serializes IRC thread lifecycle (start/stop/join/assign) so the
	   device-auth worker and the UI thread can't race on irc_thread_. */
	std::mutex irc_lifecycle_mutex_;
	std::thread irc_thread_;
	std::atomic<bool> irc_should_run_{false};
	std::atomic<bool> irc_connected_{false};
	std::atomic<intptr_t> irc_socket_{-1};
	int64_t last_chat_clip_ms_ = 0;
};

} // namespace gamepulse
