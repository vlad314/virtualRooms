#include "virtualroomsrv_listener.h"

#include <iostream>

namespace virtualroom {
namespace virtualroomsrv {

listener::listener(boost::asio::io_context &ioCtx,
                   boost::asio::ip::tcp::endpoint endpoint,
                   std::shared_ptr<connection_utils> const &conn)
    : m_acceptor(ioCtx), m_socket(ioCtx), m_conn(conn) {
  boost::beast::error_code ec;

  m_acceptor.open(endpoint.protocol(), ec);
  if (ec) {
    fail(ec, "open");
    return;
  }

  m_acceptor.bind(endpoint, ec);
  if (ec) {
    fail(ec, "bind");
    return;
  }

  m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec) {
    fail(ec, "listen");
    return;
  }

  m_conn->add_room("Project Planning");

} // namespace virtualroomsrv

void listener::accept() {
  auto socket =
      std::make_shared<boost::asio::ip::tcp::socket>(m_socket.get_io_context());
  m_acceptor.async_accept(
      m_socket,
      boost::bind(&listener::on_accept, this, socket, boost::placeholders::_1));
}

void listener::fail(boost::system::error_code ec, char const *what) {
  // Don't report on canceled operations
  if (ec == boost::asio::error::operation_aborted)
    return;
  std::cerr << what << ": " << ec.message() << "\n";
}

// void listener::on_accept(boost::beast::error_code ec) {
void listener::on_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                         boost::system::error_code ec) {
  if (ec) {
    std::cerr << "Error accepting new connection: " << ec.message() << "\n";
    return;
  }

  // std::make_shared<http_connection>(std::move(m_socket), m_conn)->run();

  m_userid = std::make_shared<std::string>("");
  // m_ids.push_back(m_userid);

  auto http_conn =
      std::make_shared<http_connection>(std::move(m_socket), m_conn);

  std::cout << " ### out for a new connection \n";
  m_http_conns_vec.push_back(http_conn);

  accept();
  // while (http_conn->m_userid == nullptr)
  //  ;
  // m_http_conns[*(http_conn->m_userid)] = http_conn;
  // std::cout << "in listener con m_userid " << *(http_conn->m_userid) << "\n";
}

} // namespace virtualroomsrv
} // namespace virtualroom