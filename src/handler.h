#pragma once

#include <memory>

namespace hera {

class Connection;

class Handler {

public:
  virtual bool OnRead(Connection* conn) = 0;
  virtual bool OnWrite(Connection* conn) = 0;

};

using HandlerPtr = std::shared_ptr<Handler>;

}  // namespace hera
