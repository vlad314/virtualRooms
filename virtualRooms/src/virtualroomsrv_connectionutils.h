#pragma once

#include <memory>
#include <string>
#include <unordered_set>

#include <boost/asio.hpp>

namespace virtualroom {
namespace virtualroomsrv {

class websocket_connection;

class connection_utils {
public:
  explicit connection_utils(std::string m_client_root);
  std::string const &client_root() const noexcept { return m_client_root; }

  void join(websocket_connection &connection);
  void leave(websocket_connection &connection);
  void send(std::string payload);
  void send_group(std::string payload, std::string room_name);
  void send_one(std::string payload, websocket_connection &connection);
  void add_id(std::string uid);
  void add_room(const std::string &room);
  void add_user_to_room(const std::string &room, const std::string &user);
  void add_user_to_room_ptr(const std::string &room,
                            websocket_connection &connection);
  void remove_user_from_room(const std::string &room, const std::string &user);
  void remove_user_from_room_ptr(const std::string &room,
                                 websocket_connection &connection);
  std::unordered_map<std::string, std::list<std::string>> &get_m_rooms();
  std::unordered_map<websocket_connection *, std::string> &get_m_conns_room();

private:
  std::vector<std::string> m_ids;
  std::unordered_map<std::string, std::list<std::string>> m_rooms;
  std::unordered_map<std::string, std::list<websocket_connection *>>
      m_rooms_ptr;
  std::unordered_map<websocket_connection *, std::string> m_conns_room;
  std::unordered_set<websocket_connection *> get_connections();
  std::string m_client_root;
  std::unordered_set<websocket_connection *> m_connections;
};

} // namespace virtualroomsrv
} // namespace virtualroom