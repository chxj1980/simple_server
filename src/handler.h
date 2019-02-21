#pragma once

#include <memory>

#include "buffer.h"

namespace hera {

class Connection;

class Handler {

public:
  enum ReqStatus {
    kReqReady = 0,
    kReqNotReady = 1,
    kReqInvalid = 2
  };

  virtual ReqStatus OnRequest(Connection* conn) = 0;
  virtual bool OnComplete(bool succ) = 0;
};

using HandlerPtr = std::shared_ptr<Handler>;

}  // namespace hera
