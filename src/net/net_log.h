// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_SHELL_NET_NET_LOG_H_
#define EXO_SHELL_NET_NET_LOG_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "net/base/net_log_logger.h"

namespace exo_shell {

class ExoShellNetLog : public net::NetLog {
 public:
  ExoShellNetLog();
  virtual ~ExoShellNetLog();

 private:
  scoped_ptr<net::NetLogLogger> net_log_logger_;

  DISALLOW_COPY_AND_ASSIGN(ExoShellNetLog);
};

} // namespace exo_shell

#endif // EXO_SHELL_NET_NET_LOG_H_
