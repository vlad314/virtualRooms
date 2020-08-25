#pragma once

#include <stdexcept>
#include <string>

namespace virtualroom {
namespace virtualroomsrv {
class Error : public std::runtime_error {
public:
  explicit Error(const std::string &msg);
};

} // namespace virtualroomsrv
} // namespace virtualroom