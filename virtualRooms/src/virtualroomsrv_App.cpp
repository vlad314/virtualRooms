
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

  // boost::asio::signal_set signals(m_ioCtx, SIGINT, SIGTERM);
  // signals.async_wait([&m_ioCtx](boost::system::error_code const &, int) {
  //  // Stop the io_context. This will cause run()
  //  // to return immediately, eventually destroying the
  //  // io_context and any remaining handlers in it.
  //  m_ioCtx.stop();
  //});

  // Run the I/O service on the main thread
  m_ioCtx.run();
}

} // namespace virtualroomsrv
} // namespace virtualroom