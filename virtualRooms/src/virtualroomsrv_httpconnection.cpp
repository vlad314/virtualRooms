#include "virtualroomsrv_httpconnection.h"
#include "virtualroomsrv_websocketconnection.h"
#include <iostream>
#include <string>

namespace virtualroom {
namespace virtualroomsrv {

boost::beast::string_view mime_type(boost::beast::string_view path) {
  using boost::beast::iequals;

  auto const ext = [&path] {
    auto const pos = path.rfind(".");
    if (pos == boost::beast::string_view::npos)
      return boost::beast::string_view{};
    return path.substr(pos);
  }();

  if (iequals(ext, ".htm"))
    return "text/html";
  if (iequals(ext, ".html"))
    return "text/html";
  if (iequals(ext, ".php"))
    return "text/html";
  if (iequals(ext, ".css"))
    return "text/css";
  if (iequals(ext, ".txt"))
    return "text/plain";
  if (iequals(ext, ".js"))
    return "application/javascript";
  if (iequals(ext, ".json"))
    return "application/json";
  if (iequals(ext, ".xml"))
    return "application/xml";
  if (iequals(ext, ".swf"))
    return "application/x-shockwave-flash";
  if (iequals(ext, ".flv"))
    return "video/x-flv";
  if (iequals(ext, ".png"))
    return "image/png";
  if (iequals(ext, ".jpe"))
    return "image/jpeg";
  if (iequals(ext, ".jpeg"))
    return "image/jpeg";
  if (iequals(ext, ".jpg"))
    return "image/jpeg";
  if (iequals(ext, ".gif"))
    return "image/gif";
  if (iequals(ext, ".bmp"))
    return "image/bmp";
  if (iequals(ext, ".ico"))
    return "image/vnd.microsoft.icon";
  if (iequals(ext, ".tiff"))
    return "image/tiff";
  if (iequals(ext, ".tif"))
    return "image/tiff";
  if (iequals(ext, ".svg"))
    return "image/svg+xml";
  if (iequals(ext, ".svgz"))
    return "image/svg+xml";
  return "application/text";
}

std::string build_client_path(boost::beast::string_view base,
                              boost::beast::string_view path) {
  if (base.empty())
    return path.to_string();

  std::string result = base.to_string();
  char constexpr path_separator = '\\';

  if (result.back() == path_separator)
    result.resize(result.size() - 1);
  result.append(path.data(), path.size());

  for (auto &c : result)
    if (c == '/')
      c = path_separator;

  return result;
}

http_connection::http_connection(boost::asio::ip::tcp::socket socket,
                                 std::shared_ptr<connection_utils> &conn)
    : m_socket(std::move(socket)), m_conn(conn) {

  boost::beast::http::async_read(m_socket, m_buffer, m_req,
                                 boost::bind(&http_connection::on_read, this,
                                             boost::placeholders::_1,
                                             boost::placeholders::_2));
}

std::string send_path(const boost::beast::string_view &client_root_path,
                      std::string &s) {
  s.replace(0, 1, "\\"); // replace "/" with "\\"
  if (s == "\\") {
    s.append("index.html");
  }
  auto path = build_client_path(client_root_path,
                                s); // append to root path
  return path;
}

template <class Body, class Allocator, class Send>
void handle_request(
    boost::beast::string_view client_root_path,
    boost::beast::http::request<
        Body, boost::beast::http::basic_fields<Allocator>> &&req,
    Send &&send) {

  std::string path;
  std::string path_uri;

  boost::beast::string_view req_uri = req.target();
  auto found_slash = req_uri.rfind("\/");

  boost::beast::string_view last_chs_path;
  last_chs_path = req_uri.substr(found_slash);

  if (last_chs_path == "\/favicon.ico") {
    std::cout << "found favicon - return\n";
    return;
  } else {
    auto found_qmark = last_chs_path.rfind("?");

    boost::beast::string_view before_qmark;
    boost::beast::string_view after_qmark;
    before_qmark = last_chs_path;

    path = "";
    std::string before_qmark_string = before_qmark.to_string();
    path.append(send_path(client_root_path, before_qmark_string));
  }

  boost::beast::error_code ec;
  boost::beast::http::file_body::value_type body;
  body.open(path.c_str(), boost::beast::file_mode::scan, ec);

  if (ec == boost::system::errc::no_such_file_or_directory)
    std::cerr << "No such file\n";

  auto const size = body.size();

  boost::beast::http::response<boost::beast::http::file_body> res{
      std::piecewise_construct, std::make_tuple(std::move(body)),
      std::make_tuple(boost::beast::http::status::ok, req.version())};

  res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
  res.set(boost::beast::http::field::content_type, mime_type(path));
  res.content_length(size);
  res.keep_alive(req.keep_alive());
  return send(std::move(res));
}

void http_connection::fail(boost::beast::error_code ec, char const *what) {
  if (ec == boost::asio::error::operation_aborted)
    return;
  std::cerr << what << ": " << ec.message() << "\n";
}

void http_connection::on_read(boost::beast::error_code ec, std::size_t) {

  if (ec == boost::beast::http::error::end_of_stream) {
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    return;
  }
  if (ec) {
    return fail(ec, "read");
  }

  if (boost::beast::websocket::is_upgrade(m_req)) {
    auto m_ws_conns =
        std::make_shared<websocket_connection>(std::move(m_socket), m_conn);
    m_ws_conns->run(std::move(m_req));

    return;
  }

  std::vector<std::pair<std::string, std::string>> payload;

  handle_request(
      m_conn->client_root(), std::move(m_req), [this](auto &&response) {
        using response_type = typename std::decay<decltype(response)>::type;
        auto sp = std::make_shared<response_type>(
            std::forward<decltype(response)>(response));

        auto self = shared_from_this();
        boost::beast::http::async_write(
            this->m_socket, *sp,
            [self, sp](boost::beast::error_code ec, std::size_t bytes) {
              self->on_write(ec, bytes, sp->need_eof());
            });
      });
}

void http_connection::on_write(boost::beast::error_code ec, std::size_t,
                               bool close) {
  if (ec)
    return fail(ec, "write");

  if (close) {
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    return;
  }

  boost::beast::http::async_read(
      m_socket, m_buffer, m_req,
      [self = shared_from_this()](boost::beast::error_code ec,
                                  std::size_t bytes) {
        self->on_read(ec, bytes);
      });
}

} // namespace virtualroomsrv
} // namespace virtualroom