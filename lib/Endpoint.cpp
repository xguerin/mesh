#include "Endpoint.h"
#include "Percentile.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <vector>
#include <ace/common/Log.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <sys/socket.h>

namespace mesh {

Endpoint::Endpoint(config::IEndpoint const & cfg, const int dur, const int sec)
  : m_duration(dur)
  , m_sec(sec)
  , m_fd(-1)
  , m_timerfd(timerfd_create(CLOCK_MONOTONIC, 0))
  , m_shutdown(false)
  , m_thread()
  , m_clients()
  , m_expected(cfg.clients())
  , m_done(false)
{
  struct sockaddr_in address;
  /*
   * Create the server socket.
   */
  if ((m_fd = socket(PF_INET, SOCK_STREAM, 0)) == 0) {
    throw std::runtime_error(strerror(errno));
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(cfg.port());
  /*
   * Force port reuse.
   */
  int flag = 1;
  if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag)) == -1) {
    ACE_LOG(Error, strerror(errno));
    throw std::runtime_error(strerror(errno));
  }
  /*
   * Bind the socket.
   */
  if (bind(m_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    ACE_LOG(Error, strerror(errno));
    throw std::runtime_error(strerror(errno));
  }
  if (listen(m_fd, 3) < 0) {
    ACE_LOG(Error, strerror(errno));
    throw std::runtime_error(strerror(errno));
  }
  /*
   * Create the runner thread.
   */
  m_thread = std::thread(&Endpoint::run, this);
  ACE_LOG(Info, "Endpoint created (", m_sec, "s), listening on port: ", cfg.port());
}

Endpoint::~Endpoint()
{
  /*
   * Shutdown the receiver thread
   */
  m_shutdown = true;
  m_thread.join();
  /*
   * Close the clients.
   */
  for (auto const & c : m_clients) {
    ::close(c);
  }
  /*
   * Close the server socket.
   */
  ::close(m_timerfd);
  ::close(m_fd);
  ACE_LOG(Info, "Endpoint destructed");
}

#define BUFFER_SIZE (1024 * 1024)

void
Endpoint::run()
{
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  uint8_t * buffer = new uint8_t[BUFFER_SIZE];
  size_t counter = 0, last = 0, delta = 0;
  std::vector<size_t> data;
  auto header = "Bytes/s (" + std::to_string(m_sec) + "s): ";
  int ticks = m_duration / m_sec;
  /*
   * Prepare the timer.
   */
  struct timespec ts = { m_sec, 0 };
  struct itimerspec its = { ts, ts };
  /*
   * Arm the timer.
   */
  if (m_expected > 0) {
    timerfd_settime(m_timerfd, 0, &its, &its);
  }
  /*
   * Runner loop.
   */
  ACE_LOG(Info, "Endpoint runner started, ticks = ", ticks);
  while (!m_shutdown) {
    /*
     * Prepare the poll descriptors.
     */
    struct pollfd fds[2 + m_clients.size()];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = m_fd;
    fds[0].events = POLLIN;
    fds[1].fd = m_timerfd;
    fds[1].events = POLLIN;
    for (size_t i = 0; i < m_clients.size(); i += 1) {
      fds[i + 2].fd = m_clients[i];
      fds[i + 2].events = POLLIN;
    }
    /*
     * Wait for a connection.
     */
    int res = poll(fds, 2 + m_clients.size(), 100);
    if (res <= 0) {
      continue;
    }
    /*
     * Process the server socket.
     */
    if (fds[0].revents & POLLIN) {
      ACE_LOG(Info, "Accepting new client connection");
      int fd = accept(m_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
      if (fd >= 0) {
        fcntl(fd, O_NONBLOCK);
        m_clients.push_back(fd);
      }
    }
    /*
     * Process the client sockets.
     */
    for (size_t i = 0; i < m_clients.size(); i += 1) {
      if (fds[i + 2].revents & POLLIN) {
        counter += read(fds[i + 2].fd, buffer, BUFFER_SIZE);
      }
    }
    /*
     * Process the timer socker.
     */
    if (fds[1].revents & POLLIN) {
      uint64_t value;
      ssize_t ret = read(m_timerfd, &value, sizeof(value));
      if (ret <= 0) continue;
      delta = (counter - last) / m_sec;
      data.push_back(delta);
      ACE_LOG(Debug, header, delta);
      last = counter;
      if (m_clients.size() == m_expected) {
        ticks -= 1;
      }
    }
    /*
     * Terminate if ticks == 0.
     */
    if (ticks == 0) {
      break;
    }
  }
  /*
   * Display the percentiles and return.
   */
  if (m_expected > 0 ) {
    Percentile<size_t> percentile(data);
    percentile.print(std::cout, 10, 20, 30, 40, 50, 60, 70, 80, 90, 95, 99);
  }
  ACE_LOG(Info, "Endpoint runner ended");
  m_done = true;
  delete[] buffer;
}

}
