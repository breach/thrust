// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_NET_NET_LOG_H_
#define THRUST_SHELL_NET_NET_LOG_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "net/base/net_log_logger.h"

namespace thrust_shell {

class ThrustShellNetLog : public net::NetLog {
 public:
  ThrustShellNetLog();
  virtual ~ThrustShellNetLog();

 private:
  scoped_ptr<net::NetLogLogger> net_log_logger_;

  DISALLOW_COPY_AND_ASSIGN(ThrustShellNetLog);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_NET_NET_LOG_H_
