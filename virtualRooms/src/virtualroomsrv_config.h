#pragma once

#include <string>

namespace virtualroom {
namespace virtualroomsrv {

struct Config {
  std::string m_addr;
  unsigned short m_port;
  std::string m_client_path;
};

} // namespace virtualroomsrv
} // namespace virtualroom