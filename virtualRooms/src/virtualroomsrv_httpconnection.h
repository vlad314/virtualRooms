#pragma once

#include "virtualroomsrv_connectionutils.h"

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <memory>

namespace virtualroom {
namespace virtualroomsrv {

class http_connection : public std::enable_shared_from_this<http_connection> {
public:
  using userId = std::string;

  http_connection(boost::asio::ip::tcp::socket socket,
                  std::shared_ptr<connection_utils> &c_utils);

private:
  boost::asio::ip::tcp::socket m_socket;
  boost::beast::flat_buffer m_buffer;
  std::shared_ptr<connection_utils> m_conn;
  boost::beast::http::request<boost::beast::http::string_body> m_req;

  void fail(boost::beast::error_code ec, char const *what);
  void on_read(boost::beast::error_code ec, std::size_t);
  void on_write(boost::beast::error_code ec, std::size_t, bool close);
};

} // namespace virtualroomsrv
} // namespace virtualroom