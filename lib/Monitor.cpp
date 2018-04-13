#include "Monitor.h"
#include <cerrno>
#include <exception>
#include <iostream>
#include <ace/common/Log.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace mesh {

Monitor::Monitor(const int sec)
  : m_sec(sec)
  , m_fd(timerfd_create(CLOCK_MONOTONIC, 0))
  , m_shutdown(false)
  , m_thread(&Monitor::run, this)
  , m_watchers()
  , m_mutex()
{

}

Monitor::~Monitor()
{
  m_shutdown = true;
  m_thread.join();
}

void
Monitor::run()
{
  struct timespec ts = { m_sec, 0 };
  struct itimerspec its = { ts, ts };
  /*
   * Arm the timer.
   */
  timerfd_settime(m_fd, 0, &its, &its);
  /*
   * Run the monitor loop.
   */
  struct pollfd fds = { m_fd, POLLIN, 0 };
  while (!m_shutdown) {
    //
    // Wait for a timer event.
    //
    int res = poll(&fds, 1, -1);
    if (res <= 0) {
      continue;
    }
    if (m_shutdown) {
      break;
    }
    //
    // Read the timer event if any.
    //
    if (res == 1) {
      uint64_t value;
      read(m_fd, & value, sizeof(value));
      fds.revents = 0;
    }
    //
    // Print the variables.
    //
    std::lock_guard<std::mutex> _(m_mutex);
    for (auto const & w: m_watchers) {
      w->print(std::cout);
    }
  }
  /*
   * Clean-up.
   */
  close(m_fd);
}

}
