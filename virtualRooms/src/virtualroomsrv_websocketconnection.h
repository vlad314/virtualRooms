#pragma once

#include "virtualroomsrv_connectionutils.h"

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace virtualroom {
namespace virtualroomsrv {

class connection_utils;

class websocket_connection
    : public std::enable_shared_from_this<websocket_connection> {

public:
  websocket_connection(boost::asio::ip::tcp::socket socket,
                       std::shared_ptr<connection_utils> &conn);
  ~websocket_connection();

  template <class Body, class Allocator>
  void
  run(boost::beast::http::request<Body,
                                  boost::beast::http::basic_fields<Allocator>>
          req);
  void send(std::shared_ptr<std::string const> const &payload);
  std::string websocket_connection::message_group(const std::string &room);
  std::string
  websocket_connection::message_chat_group(const std::string &payload,
                                           const std::string &user);
  std::string m_userid;
  // std::shared_ptr<std::string> m_userid;

private:
  boost::beast::flat_buffer m_buffer;
  boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_ws;
  std::shared_ptr<connection_utils> m_conn;
  std::vector<std::shared_ptr<std::string const>> m_queue;

  void fail(boost::beast::error_code ec, char const *what);
  void on_accept(boost::beast::error_code ec);
  void on_read(boost::beast::error_code ec, std::size_t bytes_transf);
  void on_write(boost::beast::error_code ec, std::size_t bytes_transf);
  std::string message_state();
  void change_state(std::string &msg);
  std::string m_action_code;
  std::string m_current_chat_room;
  std::string m_message;
  std::string m_current_user;
};

template <class Body, class Allocator>
void websocket_connection::run(
    boost::beast::http::request<Body,
                                boost::beast::http::basic_fields<Allocator>>
        req) {
  // Accept the websocket handshake
  m_ws.async_accept(req, std::bind(&websocket_connection::on_accept,
                                   shared_from_this(), std::placeholders::_1));
}
} // namespace virtualroomsrv
} // namespace virtualroom