
#include <virtualroomsrv_app.h>

namespace virtualroom {
namespace virtualroomsrv {

App::App(const std::string_view &cfgFile) : m_ioCtx{1} {
  auto cfg = ConfigReader::readerConfig(cfgFile);

  auto listen = std::make_shared<listener>(
      m_ioCtx,
      boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(cfg.m_addr),
                                     cfg.m_port),
      std::make_shared<connection_utils>(cfg.m_client_path));

  listen->accept();

  // Run the I/O service on the main thread
  m_ioCtx.run();
}

} // namespace virtualroomsrv
} // namespace virtualroom