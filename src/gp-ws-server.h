/*
GamePulse for OBS — embedded localhost WebSocket server (RFC 6455, server side only).
No external dependencies: raw sockets (winsock2 / BSD), vendored SHA-1 + base64.
Binds 127.0.0.1 only. Text frames carry one JSON message each.
*/

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace gamepulse {

class WsServer {
public:
	/* Handlers are invoked from per-client reader threads — marshal to the
	   OBS main thread yourself before touching libobs/Qt state. */
	using MessageHandler = std::function<void(int client_id, const std::string &text)>;
	using ConnectHandler = std::function<void(int client_id, const std::string &remote)>;
	using DisconnectHandler = std::function<void(int client_id)>;

	WsServer();
	~WsServer();

	WsServer(const WsServer &) = delete;
	WsServer &operator=(const WsServer &) = delete;

	/* Start listening on 127.0.0.1:port. If token is non-empty, clients must
	   present it either as ?token=... in the request path or in a
	   "Authorization: Bearer ..." header during the handshake. */
	bool start(uint16_t port, const std::string &token);
	void stop();
	bool running() const;
	uint16_t port() const;

	void send_text(int client_id, const std::string &text);
	void broadcast_text(const std::string &text);
	void close_client(int client_id);
	int client_count() const;

	void set_on_message(MessageHandler h);
	void set_on_connect(ConnectHandler h);
	void set_on_disconnect(DisconnectHandler h);

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

} // namespace gamepulse
