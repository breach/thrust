// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_NET_NET_LOG_H_
#define EXO_BROWSER_NET_NET_LOG_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "net/base/net_log_logger.h"

namespace exo_browser {

class ExoBrowserNetLog : public net::NetLog {
 public:
  ExoBrowserNetLog();
  virtual ~ExoBrowserNetLog();

 private:
  scoped_ptr<net::NetLogLogger> net_log_logger_;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserNetLog);
};

} // namespace exo_browser

#endif // EXO_BROWSER_NET_NET_LOG_H_
