#pragma once

#include "virtualroomsrv_config.h"

#include <string_view>

namespace virtualroom {
namespace virtualroomsrv {

class ConfigReader {
public:
  static Config readerConfig(const std::string_view &file);
};

} // namespace virtualroomsrv
} // namespace virtualroom