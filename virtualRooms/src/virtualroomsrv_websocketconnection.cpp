#include "virtualroomsrv_websocketconnection.h"

namespace virtualroom {
namespace virtualroomsrv {

websocket_connection::websocket_connection(
    boost::asio::ip::tcp::socket socket,
    std::shared_ptr<connection_utils> &conn)
    : m_ws(std::move(socket)), m_conn(conn) {}

websocket_connection::~websocket_connection() {

  auto room = m_conn->get_m_conns_room()[this];
  m_conn->remove_user_from_room_ptr(room, *this);

  m_conn->leave(*this);
}

void websocket_connection::change_state(std::string &msg) {

  if (msg != "reqid") {
    std::vector<std::string> keys;
    std::vector<std::string> values;
    auto colon = msg.find(":");
    auto amp = msg.find("&");
    auto first_eq = (msg.substr(0, amp)).find("=");
    std::string tmp_amp = msg.substr(amp);
    auto second_eq = tmp_amp.find("=");
    std::string code = msg.substr(0, colon);
    keys.push_back(msg.substr(colon + 1, first_eq - colon - 1));
    values.push_back(msg.substr(first_eq + 1, amp - first_eq - 1));
    keys.push_back(tmp_amp.substr(1, second_eq - 1));
    values.push_back(tmp_amp.substr(second_eq + 1));

    m_action_code = code;

    if (code == "join") {
      m_conn->add_user_to_room(values[1], values[0]);
      m_conn->add_user_to_room_ptr(values[1], *this);
      m_current_chat_room = values[1];
      std::cout << "joined room\n";
    } else if (code == "create") {
      m_conn->add_room(values[1]);
      m_current_chat_room = values[1];
    } else if (code == "chat") {
      m_message = values[1];
      m_current_user = values[0];
    }
  } else {
    m_userid = "id";
  }
}

std::string websocket_connection::message_state() {
  std::string msg;
  msg.append("rooms:");
  for (auto r : m_conn->get_m_rooms()) {
    msg.append(r.first);
    msg.append("{");
    for (auto u : r.second) {
      msg.append(u);
      msg.append(";");
    }
    if (msg.back() == ';')
      msg.pop_back();
    msg.append("}");
    msg.append("/");
  }
  if (msg.back() == '/')
    msg.pop_back();
  return msg;
}

std::string websocket_connection::message_group(const std::string &room) {
  std::string msg;
  msg.append("joinchat");
  msg.append(":");
  msg.append(room);
  return msg;
}

std::string websocket_connection::message_chat_group(const std::string &payload,
                                                     const std::string &user) {
  std::string msg;
  msg.append("chat");
  msg.append(":");
  msg.append(user);
  msg.append("=");
  msg.append(payload);
  return msg;
}

void websocket_connection::fail(boost::beast::error_code ec, char const *what) {
  if (ec == boost::asio::error::operation_aborted ||
      ec == boost::beast::websocket::error::closed)
    return;

  std::cerr << what << ": " << ec.message() << "\n";
}

void websocket_connection::on_accept(boost::beast::error_code ec) {
  if (ec)
    return fail(ec, "accept");

  m_conn->join(*this);

  m_ws.async_read(m_buffer, [sp = shared_from_this()](
                                boost::beast::error_code ec,
                                std::size_t bytes) { sp->on_read(ec, bytes); });
}

void websocket_connection::on_read(boost::beast::error_code ec, std::size_t) {
  if (ec)
    return fail(ec, "read");

  change_state(boost::beast::buffers_to_string(m_buffer.data()));
  auto msg = message_state();

  if (m_action_code != "chat") {
    m_conn->send(msg); // send global state udpate

    if (m_action_code == "join") {
      auto msg_group = message_group(m_current_chat_room); // send joinchat

      m_conn->send_one(msg_group, *this);
    }

  } else {

    auto msg_group = message_chat_group(m_message, m_current_user);
    m_conn->send_group(msg_group, m_current_chat_room);
  }

  // Clear the buffer
  m_buffer.consume(m_buffer.size());
  std::cout << "into websocket - read again after I sent \n";

  m_ws.async_read(m_buffer, [sp = shared_from_this()](
                                boost::beast::error_code ec,
                                std::size_t bytes) { sp->on_read(ec, bytes); });
}

void websocket_connection::send(
    std::shared_ptr<std::string const> const &payload) {

  m_queue.push_back(payload);

  if (m_queue.size() > 1)
    return;

  m_ws.async_write(boost::asio::buffer(*m_queue.front()),
                   [sp = shared_from_this()](boost::beast::error_code ec,
                                             std::size_t bytes) {
                     sp->on_write(ec, bytes);
                   });
}

void websocket_connection::on_write(boost::beast::error_code ec, std::size_t) {
  if (ec)
    return fail(ec, "write");

  m_queue.erase(m_queue.begin());

  if (!m_queue.empty())
    m_ws.async_write(boost::asio::buffer(*m_queue.front()),
                     [sp = shared_from_this()](boost::beast::error_code ec,
                                               std::size_t bytes) {
                       sp->on_write(ec, bytes);
                     });
}

} // namespace virtualroomsrv
} // namespace virtualroom
