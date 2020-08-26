#include "virtualroomsrv_connectionutils.h"
#include "virtualroomsrv_httpconnection.h"
#include "virtualroomsrv_websocketconnection.h"

namespace virtualroom {
namespace virtualroomsrv {

connection_utils::connection_utils(std::string client_root)
    : m_client_root{std::move(client_root)} {}

void connection_utils::join(websocket_connection &conn) {
  m_connections.insert(&conn);
}

void connection_utils::leave(websocket_connection &conn) {
  m_connections.erase(&conn);
}

void connection_utils::send(std::string payload) {
  auto const p = std::make_shared<std::string const>(std::move(payload));
  for (auto connection : m_connections)
    connection->send(p);
}

void connection_utils::send_group(std::string payload, std::string room_name) {
  auto const p = std::make_shared<std::string const>(std::move(payload));
  auto connections = m_rooms_ptr[room_name];

  for (auto connection : connections)
    connection->send(p);
}

void connection_utils::send_one(std::string payload,
                                websocket_connection &conn) {
  auto const p = std::make_shared<std::string const>(std::move(payload));
  conn.send(p);
}

void connection_utils::add_id(std::string uid) { m_ids.push_back(uid); }

void connection_utils::add_room(const std::string &room) {
  m_rooms[room] = {};
  m_rooms_ptr[room] = {};
}

void connection_utils::add_user_to_room(const std::string &room,
                                        const std::string &user) {
  m_rooms[room].push_back(user);
}

void connection_utils::add_user_to_room_ptr(const std::string &room,
                                            websocket_connection &user) {
  m_rooms_ptr[room].push_back(&user);
  m_conns_room[&user] = room;
}

void connection_utils::remove_user_from_room(const std::string &room,
                                             const std::string &user) {
  m_rooms[room].remove(user);
}

void connection_utils::remove_user_from_room_ptr(const std::string &room,
                                                 websocket_connection &user) {
  m_rooms_ptr[room].remove(&user);
  m_conns_room[&user] = "";
}

std::unordered_set<websocket_connection *> connection_utils::get_connections() {
  return m_connections;
}

std::unordered_map<std::string, std::list<std::string>> &
connection_utils::get_m_rooms() {
  return m_rooms;
}

std::unordered_map<websocket_connection *, std::string> &
connection_utils::get_m_conns_room() {
  return m_conns_room;
}

} // namespace virtualroomsrv
} // namespace virtualroom