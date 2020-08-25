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
  std::cout << " ### in http constructor  "
            << "\n";
  if (m_userid != nullptr) {
    std::cout << " ### in http constructor  " << *m_userid << "\n";
  }
  boost::beast::http::async_read(m_socket, m_buffer, m_req,
                                 boost::bind(&http_connection::on_read, this,
                                             boost::placeholders::_1,
                                             boost::placeholders::_2));
}

// template <class Response>
// std::shared_ptr<boost::beast::http::response<boost::beast::http::file_body>>
// send_resp(const Response &res_body) {
//  boost::beast::http::response<boost::beast::http::file_body> res{
//      std::piecewise_construct, std::make_tuple(std::move(res_body)),
//      std::make_tuple(boost::beast::http::status::ok, req.version())};
//
//  res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
//  res.set(boost::beast::http::field::content_type, mime_type(path));
//  res.content_length(size);
//  res.keep_alive(req.keep_alive());
//  return std::make_shared<
//      boost::beast::http::response<boost::beast::http::file_body>>(res);
//}

std::string send_path(const boost::beast::string_view &client_root_path,
                      std::string &s) {
  s.replace(0, 1, "\\"); // replace "/" with "\\"
  std::cout << "s = " << s << "\n";
  if (s == "\\") {
    s.append("index.html");
    std::cout << "s index = " << s << "\n";
  }
  auto path = build_client_path(client_root_path,
                                s); // append to root path
  std::cout << "path = " << path << "\n";
  return path;
}

template <class Body, class Allocator, class Send, class Resp, class userid>
void handle_request(
    boost::beast::string_view client_root_path,
    boost::beast::http::request<
        Body, boost::beast::http::basic_fields<Allocator>> &&req,
    Send &&send, Resp &payload, userid &uid) {

  std::cout << "------ into handle_request\n";
  std::cout << "req_uri = " << req.target() << "\n";
  // std::cout << "req_uri[1] = " << req.target()[1] << "\n";

  // std::string req_uri(req.target().end() - 5, req.target().end());
  // favicon.ico
  std::string path;
  std::string path_uri;

  boost::beast::string_view req_uri = req.target();
  auto found_slash = req_uri.rfind("\/");

  boost::beast::string_view last_chs_path;
  last_chs_path = req_uri.substr(found_slash);
  std::cout << "last_chs_path = " << last_chs_path << "\n";

  if (last_chs_path == "\/favicon.ico") {
    std::cout << "found favicon - return\n";
    return;
  } else {
    auto found_qmark = last_chs_path.rfind("?");
    if (found_qmark ==
        boost::beast::string_view::npos) { // if no form data is sent

      path = "";
      path_uri = last_chs_path.to_string();
      path.append(send_path(client_root_path, path_uri));
      std::cout << "path index = " << path << "\n";

    } else { // if form data is sent as a GET req, identify the "?" and send the
             // html before the "?"
      boost::beast::string_view before_qmark;
      boost::beast::string_view after_qmark;
      before_qmark = last_chs_path.substr(0, found_qmark);

      after_qmark = last_chs_path.substr(found_qmark + 1);
      std::cout << "before_qmark = " << before_qmark << "\n";
      std::cout << "after_qmark = " << after_qmark << "\n";

      std::string after_qmark_string = after_qmark.to_string();
      auto amp_index = after_qmark_string.find("&");

      while (amp_index != std::string::npos) {
        auto equal_index = after_qmark_string.find("=");

        std::string req_key = after_qmark_string.substr(0, equal_index);
        std::cout << "key = " << req_key << "\n";
        std::string req_value = after_qmark_string.substr(
            0 + equal_index + 1, amp_index - equal_index);
        std::cout << "value = " << req_value << "\n";

        payload.push_back(
            std::pair<std::string, std::string>(req_key, req_value));

        after_qmark_string.replace(after_qmark_string.begin(),
                                   after_qmark_string.end(),
                                   after_qmark_string.substr(amp_index + 1));

        if (after_qmark_string.find('&') == std::string::npos)
          break;
      }

      auto equal_index = after_qmark_string.find("=");
      std::string req_key;
      std::string req_value;
      if (equal_index != std::string::npos) {
        req_key = after_qmark_string.substr(0, equal_index);
        std::cout << "key = " << req_key << "\n";
        req_value = after_qmark_string.substr(equal_index + 1);
        std::cout << "value = " << req_value << "\n";
        payload.push_back(
            std::pair<std::string, std::string>(req_key, req_value));
        std::cout << " [2]after payl in handle_req\n";
      }

      /*  if (!payload.empty() && payload[0].first == "userid") {
          *uid = payload[0].second;
          std::cout << " -- uid handle_req : " << *uid << "\n";
        }*/
      std::cout << " [4]after payl in handle_req\n";

      // payload.push_back(
      //    std::pair<std::string, std::string>(req_key, req_value));

      // std::unordered_map<std::string, std::string> payload;
      // std::string req_key_string = req_key.to_string();
      // std::string req_value_string = req_value.to_string();

      // m_http_conns[req_value_string]

      path = "";
      std::string before_qmark_string = before_qmark.to_string();
      path.append(send_path(client_root_path, before_qmark_string));
    }
  }

  std::cout << "path = " << path << "\n";

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
} // namespace virtualroomsrv

void http_connection::fail(boost::beast::error_code ec, char const *what) {
  if (ec == boost::asio::error::operation_aborted)
    return;
  std::cerr << what << ": " << ec.message() << "\n";
}

void http_connection::on_read(boost::beast::error_code ec, std::size_t) {

  std::cout << "\n -- into http_session.on_read()1\n";
  // std::cout << " into on_read 1   : " << *uid << "\n";
  if (ec == boost::beast::http::error::end_of_stream) {
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    return;
  }
  // std::cout << "into http_session.on_read() 1 \n";
  if (ec) {
    return fail(ec, "read");
  }
  // std::cout << "into http_session.on_read() 2 \n";

  // std::cout << "after payl on_read 1 : " << *m_userid << "\n";

  // when press websocket connect
  if (boost::beast::websocket::is_upgrade(m_req)) {
    std::cout << "into http_session.on_read.upgrade()\n";
    // std::cout << "ws payl : " << *m_userid << "\n";
    auto m_ws_conns_vec =
        std::make_shared<websocket_connection>(std::move(m_socket), m_conn);
    m_ws_conns_vec->run(std::move(m_req));

    return;
  }

  // std::shared_ptr<std::unordered_map<std::string, std::string>> post_body;
  // std::shared_ptr<std::vector<std::pair<std::string, std::string>>> payload;
  std::vector<std::pair<std::string, std::string>> payload;

  // std::cout << "into http_session.on_read() 2 \n";
  handle_request(
      m_conn->client_root(), std::move(m_req),
      [this](auto &&response) {
        using response_type = typename std::decay<decltype(response)>::type;
        auto sp = std::make_shared<response_type>(
            std::forward<decltype(response)>(response));

        auto self = shared_from_this();
        boost::beast::http::async_write(
            this->m_socket, *sp,
            [self, sp](boost::beast::error_code ec, std::size_t bytes) {
              self->on_write(ec, bytes, sp->need_eof());
              // std::cout << " --- in lambda = " << *uid << "\n";
            });
      },
      payload, m_userid);

  if (!payload.empty() && payload[0].first == "reqid") {

    std::cout << "payload[0].first = " << payload[0].first << "\n"
              << "payload[0].second = " << payload[0].second << "\n";
    //*uid = payload[0].second;
    m_conn->add_id(payload[0].second);
    std::cout << " [1122] \n";
    m_userid = std::make_shared<std::string>("");
    *m_userid = m_conn->m_ids.back();
    std::cout << "after payl : " << *m_userid << "\n";
  }

  // std::cout << "before\n";
  //*m_userid = "zxc";
  // std::cout << "into http_session.on_read()3\n";
} // namespace virtualroomsrv

void http_connection::on_write(boost::beast::error_code ec, std::size_t,
                               bool close) {
  std::cout << "int on_write "
            << "\n";
  if (ec)
    return fail(ec, "write");

  if (close) {
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    return;
  }
  // std::cout << "-- uid in on_write : " << *uid << "\n";
  boost::beast::http::async_read(
      m_socket, m_buffer, m_req,
      [self = shared_from_this()](boost::beast::error_code ec,
                                  std::size_t bytes) {
        // std::cout << " into on_write lambda : " << *uid << "\n";
        self->on_read(ec, bytes);
      });
}

} // namespace virtualroomsrv
} // namespace virtualroom