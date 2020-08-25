#pragma once

#include "virtualroomsrv_connectionutils.h"
#include "virtualroomsrv_httpconnection.h"

#include <boost/bind.hpp>
#include <memory>
#include <string>

namespace virtualroom {
namespace virtualroomsrv {

class listener : public std::enable_shared_from_this<listener> {
public:
  listener(boost::asio::io_context &ioCtx,
           boost::asio::ip::tcp::endpoint endpoint,
           std::shared_ptr<connection_utils> const &conn);
  void accept();
  std::shared_ptr<std::string> m_userid;

private:
  boost::asio::ip::tcp::acceptor m_acceptor;
  boost::asio::ip::tcp::socket m_socket;
  std::shared_ptr<connection_utils> m_conn;

  void fail(boost::beast::error_code ec, char const *what);
  void on_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                 boost::system::error_code ec);
  std::vector<std::shared_ptr<http_connection>> m_http_conns_vec;
  std::unordered_map<std::string, std::shared_ptr<http_connection>>
      m_http_conns;
  std::unordered_map<std::string, std::vector<std::shared_ptr<std::string>>>
      m_rooms_vec;
};

} // namespace virtualroomsrv
} // namespace virtualroom