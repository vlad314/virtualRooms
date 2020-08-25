#pragma once

#include <virtualroomsrv_configreader.h>
#include <virtualroomsrv_listener.h>

#include <boost/asio.hpp>

namespace virtualroom {
namespace virtualroomsrv {

class App {
public:
  explicit App(const std::string_view &cfgFile);
  int run();

private:
  boost::asio::io_context m_ioCtx;
};

} // namespace virtualroomsrv
} // namespace virtualroom