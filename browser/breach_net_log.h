// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_BROWSER_BREACH_NET_LOG_H_
#define BREACH_BROWSER_BREACH_NET_LOG_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "net/base/net_log_logger.h"

namespace breach {

class BreachNetLog : public net::NetLog {
 public:
  BreachNetLog();
  virtual ~BreachNetLog();

 private:
  scoped_ptr<net::NetLogLogger> net_log_logger_;

  DISALLOW_COPY_AND_ASSIGN(BreachNetLog);
};

} // namespace breach

#endif // BREACH_BROWSER_NET_LOG_H_
