#include "virtualroomsrv_websocketconnection.h"

namespace virtualroom {
namespace virtualroomsrv {

websocket_connection::websocket_connection(
    boost::asio::ip::tcp::socket socket,
    std::shared_ptr<connection_utils> &conn)
    : m_ws(std::move(socket)),
      m_conn(conn) { //, m_action_code(""), m_message("") {
  // if (m_userid != nullptr) {
  //  std::cout << "in webs con m_userid " << *m_userid << "\n";
  //}
}

websocket_connection::~websocket_connection() {
  std::cout << " ~~~ destruct ws\n";
  auto room = m_conn->m_conns_room[this];
  m_conn->remove_user_from_room_ptr(room, *this);

  // auto msg = message_state();

  // std::cout << "leave msg  ... \n";
  //// m_conn->send(msg);
  // std::string leave_msg = "leave";
  // leave_msg = leave_msg.append(":");
  // leave_msg = leave_msg.append(msg);
  // m_queue.push_back(std::make_shared<std::string const>(leave_msg));

  // std::cout << "done leave msg  ... \n";

  // while (m_queue.size() > 1)
  //  ;
  //// return;

  // std::cout << "into destructor send() - sending \n";
  // std::shared_ptr<std::string> leave;
  // leave = std::make_shared<std::string>("false");

  //  m_ws.async_write(
  //      boost::asio::buffer(*m_queue.front()),
  //      [&](boost::beast::error_code ec, std::size_t bytes) {
  //        m_buffer.consume(m_buffer.size());
  //        m_ws.async_read(
  //            m_buffer, [&](boost::beast::error_code ec, std::size_t bytes) {
  //              *leave = boost::beast::buffers_to_string(m_buffer.data());
  //            });
  //      });
  //
  // std::cout << "about to leave ... \n";
  // while (*leave == "false")
  //  ;
  // std::cout << "now leaving ... \n";
   m_conn->leave(*this);
}

void websocket_connection::change_state(std::string &msg) {

  if (msg != "reqid") {
    std::vector<std::string> keys;
    std::vector<std::string> values;
    std::cout << "msg from frontend = " << msg << "\n";
    auto colon = msg.find(":");
    auto amp = msg.find("&");
    auto first_eq = (msg.substr(0, amp)).find("=");
    std::string tmp_amp = msg.substr(amp);
    auto second_eq = tmp_amp.find("=");
    std::string code = msg.substr(0, colon);
    std::cout << "msg code = " << code << "\n";
    keys.push_back(msg.substr(colon + 1, first_eq - colon - 1));
    std::cout << "msg keys[0] = " << keys[0] << "\n";
    values.push_back(msg.substr(first_eq + 1, amp - first_eq - 1));
    std::cout << "msg values[0] = " << values[0] << "\n";
    keys.push_back(tmp_amp.substr(1, second_eq - 1));
    std::cout << "msg keys[1] = " << keys[1] << "\n";
    values.push_back(tmp_amp.substr(second_eq + 1));
    std::cout << "msg values[1] = " << values[1] << "\n";

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
  for (auto r : m_conn->m_rooms) {
    std::cout << "rooms are : " << r.first << "\n";
    msg.append(r.first);
    msg.append("{");
    for (auto u : r.second) {
      std::cout << "users are : " << u << "\n";
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
  std::cout << "msg to send ws is: " << msg << "\n";
  return msg;
}

std::string websocket_connection::message_group(const std::string &room) {
  std::string msg;
  msg.append("joinchat");
  msg.append(":");
  msg.append(room);
  std::cout << "join msg = " << m_current_chat_room << "\n";
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

  std::cout << "into websocket.on_accept() \n";

  // Add this session to the list of active sessions
  m_conn->join(*this);

  // Read a message
  m_ws.async_read(m_buffer, [sp = shared_from_this()](
                                boost::beast::error_code ec,
                                std::size_t bytes) { sp->on_read(ec, bytes); });
}

void websocket_connection::on_read(boost::beast::error_code ec, std::size_t) {
  // Handle the error, if any
  if (ec)
    return fail(ec, "read");

  change_state(boost::beast::buffers_to_string(m_buffer.data()));
  auto msg = message_state();
  // recevie some change_code
  // update state in conn_utils
  // send state: rooms, users in rooms, my username

  std::cout << "into websocket.on_read() \n";

  /* if (boost::beast::buffers_to_string(m_buffer.data()) == "reqid") {
     if (m_conn != nullptr && !(m_conn->m_ids).empty()) {
           auto sendp = (m_conn->m_ids).back();
           std::cout << "ws payl is : " << sendp << "\n";
           m_conn->send_one(sendp, *this);
     }
   } else {*/

  // Send to all connections
  // m_conn->send(boost::beast::buffers_to_string(m_buffer.data()));
  std::cout << "sending msg ..." << msg << " \n";
  if (m_action_code != "chat") {
    m_conn->send(msg); // send global state udpate

    // if code=join, create an extra message sent to a selected group
    std::cout << "current room = " << m_current_chat_room << "\n";
    if (m_action_code == "join") {
      auto msg_group = message_group(m_current_chat_room); // send joinchat
      m_conn->send_one(msg_group, *this);
    }

  } else {

    auto msg_group = message_chat_group(m_message, m_current_user);
    m_conn->send_group(msg_group, m_current_chat_room);
  }

  //}

  // Clear the buffer
  m_buffer.consume(m_buffer.size());
  std::cout << "into websocket - read again after I sent \n";

  // Read another message
  m_ws.async_read(m_buffer, [sp = shared_from_this()](
                                boost::beast::error_code ec,
                                std::size_t bytes) { sp->on_read(ec, bytes); });
}

void websocket_connection::send(
    std::shared_ptr<std::string const> const &payload) {

  m_queue.push_back(payload);

  std::cout << "into websocket.send() \n";

  if (m_queue.size() > 1)
    return;

  std::cout << "into websocket.send() - sending \n";

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

  std::cout << "into websocket.on_write() - send\n";

  if (!m_queue.empty())
    m_ws.async_write(boost::asio::buffer(*m_queue.front()),
                     [sp = shared_from_this()](boost::beast::error_code ec,
                                               std::size_t bytes) {
                       sp->on_write(ec, bytes);
                     });
  std::cout << "into websocket.on_write() - get out\n";
}

} // namespace virtualroomsrv
} // namespace virtualroom
