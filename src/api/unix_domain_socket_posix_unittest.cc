// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstring>
#include <queue>
#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/posix/eintr_wrapper.h"
#include "base/synchronization/condition_variable.h"
#include "base/synchronization/lock.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread.h"
#include "net/socket/socket_descriptor.h"
#include "net/socket/unix_domain_socket_posix.h"
#include "testing/gtest/include/gtest/gtest.h"

using std::queue;
using std::string;

namespace net {
namespace {

const char kSocketFilename[] = "unix_domain_socket_for_testing";
const char kInvalidSocketPath[] = "/invalid/path";
const char kMsg[] = "hello";

enum EventType {
  EVENT_ACCEPT,
  EVENT_AUTH_DENIED,
  EVENT_AUTH_GRANTED,
  EVENT_CLOSE,
  EVENT_LISTEN,
  EVENT_READ,
};

string MakeSocketPath(const string& socket_file_name) {
  base::FilePath temp_dir;
  base::GetTempDir(&temp_dir);
  return temp_dir.Append(socket_file_name).value();
}

string MakeSocketPath() {
  return MakeSocketPath(kSocketFilename);
}

class EventManager : public base::RefCounted<EventManager> {
 public:
  EventManager() : condition_(&mutex_) {}

  bool HasPendingEvent() {
    base::AutoLock lock(mutex_);
    return !events_.empty();
  }

  void Notify(EventType event) {
    base::AutoLock lock(mutex_);
    events_.push(event);
    condition_.Broadcast();
  }

  EventType WaitForEvent() {
    base::AutoLock lock(mutex_);
    while (events_.empty())
      condition_.Wait();
    EventType event = events_.front();
    events_.pop();
    return event;
  }

 private:
  friend class base::RefCounted<EventManager>;
  virtual ~EventManager() {}

  queue<EventType> events_;
  base::Lock mutex_;
  base::ConditionVariable condition_;
};

class TestListenSocketDelegate : public StreamListenSocket::Delegate {
 public:
  explicit TestListenSocketDelegate(
      const scoped_refptr<EventManager>& event_manager)
      : event_manager_(event_manager) {}

  virtual void DidAccept(StreamListenSocket* server,
                         scoped_ptr<StreamListenSocket> connection) OVERRIDE {
    LOG(ERROR) << __PRETTY_FUNCTION__;
    connection_ = connection.Pass();
    Notify(EVENT_ACCEPT);
  }

  virtual void DidRead(StreamListenSocket* connection,
                       const char* data,
                       int len) OVERRIDE {
    {
      base::AutoLock lock(mutex_);
      DCHECK(len);
      data_.assign(data, len - 1);
    }
    Notify(EVENT_READ);
  }

  virtual void DidClose(StreamListenSocket* sock) OVERRIDE {
    Notify(EVENT_CLOSE);
  }

  void OnListenCompleted() {
    Notify(EVENT_LISTEN);
  }

  string ReceivedData() {
    base::AutoLock lock(mutex_);
    return data_;
  }

 private:
  void Notify(EventType event) {
    event_manager_->Notify(event);
  }

  const scoped_refptr<EventManager> event_manager_;
  scoped_ptr<StreamListenSocket> connection_;
  base::Lock mutex_;
  string data_;
};

bool UserCanConnectCallback(
    bool allow_user, const scoped_refptr<EventManager>& event_manager,
    uid_t, gid_t) {
  event_manager->Notify(
      allow_user ? EVENT_AUTH_GRANTED : EVENT_AUTH_DENIED);
  return allow_user;
}

class UnixDomainSocketTestHelper : public testing::Test {
 public:
  void CreateAndListen() {
    socket_ = UnixDomainSocket::CreateAndListen(
        file_path_.value(), socket_delegate_.get(), MakeAuthCallback());
    socket_delegate_->OnListenCompleted();
  }

 protected:
  UnixDomainSocketTestHelper(const string& path, bool allow_user)
      : file_path_(path),
        allow_user_(allow_user) {}

  virtual void SetUp() OVERRIDE {
    event_manager_ = new EventManager();
    socket_delegate_.reset(new TestListenSocketDelegate(event_manager_));
    DeleteSocketFile();
  }

  virtual void TearDown() OVERRIDE {
    DeleteSocketFile();
    socket_.reset();
    socket_delegate_.reset();
    event_manager_ = NULL;
  }

  UnixDomainSocket::AuthCallback MakeAuthCallback() {
    return base::Bind(&UserCanConnectCallback, allow_user_, event_manager_);
  }

  void DeleteSocketFile() {
    ASSERT_FALSE(file_path_.empty());
    base::DeleteFile(file_path_, false /* not recursive */);
  }

  SocketDescriptor CreateClientSocket() {
    const SocketDescriptor sock = CreatePlatformSocket(PF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
      LOG(ERROR) << "socket() error";
      return kInvalidSocket;
    }
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    socklen_t addr_len;
    strncpy(addr.sun_path, file_path_.value().c_str(), sizeof(addr.sun_path));
    addr_len = sizeof(sockaddr_un);
    if (connect(sock, reinterpret_cast<sockaddr*>(&addr), addr_len) != 0) {
      LOG(ERROR) << "connect() error";
      return kInvalidSocket;
    }
    return sock;
  }

  scoped_ptr<base::Thread> CreateAndRunServerThread() {
    base::Thread::Options options;
    options.message_loop_type = base::MessageLoop::TYPE_IO;
    scoped_ptr<base::Thread> thread(new base::Thread("socketio_test"));
    thread->StartWithOptions(options);
    thread->message_loop()->PostTask(
        FROM_HERE,
        base::Bind(&UnixDomainSocketTestHelper::CreateAndListen,
                   base::Unretained(this)));
    return thread.Pass();
  }

  const base::FilePath file_path_;
  const bool allow_user_;
  scoped_refptr<EventManager> event_manager_;
  scoped_ptr<TestListenSocketDelegate> socket_delegate_;
  scoped_ptr<UnixDomainSocket> socket_;
};

class UnixDomainSocketTest : public UnixDomainSocketTestHelper {
 protected:
  UnixDomainSocketTest()
      : UnixDomainSocketTestHelper(MakeSocketPath(), true /* allow user */) {}
};

class UnixDomainSocketTestWithInvalidPath : public UnixDomainSocketTestHelper {
 protected:
  UnixDomainSocketTestWithInvalidPath()
      : UnixDomainSocketTestHelper(kInvalidSocketPath, true) {}
};

class UnixDomainSocketTestWithForbiddenUser
    : public UnixDomainSocketTestHelper {
 protected:
  UnixDomainSocketTestWithForbiddenUser()
      : UnixDomainSocketTestHelper(MakeSocketPath(), false /* forbid user */) {}
};

TEST_F(UnixDomainSocketTest, CreateAndListen) {
  CreateAndListen();
  EXPECT_FALSE(socket_.get() == NULL);
}

TEST_F(UnixDomainSocketTestWithInvalidPath, CreateAndListenWithInvalidPath) {
  CreateAndListen();
  EXPECT_TRUE(socket_.get() == NULL);
}

#ifdef SOCKET_ABSTRACT_NAMESPACE_SUPPORTED
// Test with an invalid path to make sure that the socket is not backed by a
// file.
TEST_F(UnixDomainSocketTestWithInvalidPath,
       CreateAndListenWithAbstractNamespace) {
  socket_ = UnixDomainSocket::CreateAndListenWithAbstractNamespace(
      file_path_.value(), "", socket_delegate_.get(), MakeAuthCallback());
  EXPECT_FALSE(socket_.get() == NULL);
}

TEST_F(UnixDomainSocketTest, TestFallbackName) {
  scoped_ptr<UnixDomainSocket> existing_socket =
      UnixDomainSocket::CreateAndListenWithAbstractNamespace(
          file_path_.value(), "", socket_delegate_.get(), MakeAuthCallback());
  EXPECT_FALSE(existing_socket.get() == NULL);
  // First, try to bind socket with the same name with no fallback name.
  socket_ =
      UnixDomainSocket::CreateAndListenWithAbstractNamespace(
          file_path_.value(), "", socket_delegate_.get(), MakeAuthCallback());
  EXPECT_TRUE(socket_.get() == NULL);
  // Now with a fallback name.
  const char kFallbackSocketName[] = "unix_domain_socket_for_testing_2";
  socket_ = UnixDomainSocket::CreateAndListenWithAbstractNamespace(
      file_path_.value(),
      MakeSocketPath(kFallbackSocketName),
      socket_delegate_.get(),
      MakeAuthCallback());
  EXPECT_FALSE(socket_.get() == NULL);
}
#endif

TEST_F(UnixDomainSocketTest, TestWithClient) {
  const scoped_ptr<base::Thread> server_thread = CreateAndRunServerThread();
  EventType event = event_manager_->WaitForEvent();
  ASSERT_EQ(EVENT_LISTEN, event);

  // Create the client socket.
  const SocketDescriptor sock = CreateClientSocket();
  ASSERT_NE(kInvalidSocket, sock);
  event = event_manager_->WaitForEvent();
  ASSERT_EQ(EVENT_AUTH_GRANTED, event);
  event = event_manager_->WaitForEvent();
  ASSERT_EQ(EVENT_ACCEPT, event);

  // Send a message from the client to the server.
  ssize_t ret = HANDLE_EINTR(send(sock, kMsg, sizeof(kMsg), 0));
  ASSERT_NE(-1, ret);
  ASSERT_EQ(sizeof(kMsg), static_cast<size_t>(ret));
  event = event_manager_->WaitForEvent();
  ASSERT_EQ(EVENT_READ, event);
  ASSERT_EQ(kMsg, socket_delegate_->ReceivedData());

  // Close the client socket.
  ret = IGNORE_EINTR(close(sock));
  event = event_manager_->WaitForEvent();
  ASSERT_EQ(EVENT_CLOSE, event);
}

TEST_F(UnixDomainSocketTestWithForbiddenUser, TestWithForbiddenUser) {
  const scoped_ptr<base::Thread> server_thread = CreateAndRunServerThread();
  EventType event = event_manager_->WaitForEvent();
  ASSERT_EQ(EVENT_LISTEN, event);
  const SocketDescriptor sock = CreateClientSocket();
  ASSERT_NE(kInvalidSocket, sock);

  event = event_manager_->WaitForEvent();
  ASSERT_EQ(EVENT_AUTH_DENIED, event);

  // Wait until the file descriptor is closed by the server.
  struct pollfd poll_fd;
  poll_fd.fd = sock;
  poll_fd.events = POLLIN;
  poll(&poll_fd, 1, -1 /* rely on GTest for timeout handling */);

  // Send() must fail.
  ssize_t ret = HANDLE_EINTR(send(sock, kMsg, sizeof(kMsg), 0));
  ASSERT_EQ(-1, ret);
  ASSERT_EQ(EPIPE, errno);
  ASSERT_FALSE(event_manager_->HasPendingEvent());
}

}  // namespace
}  // namespace net
