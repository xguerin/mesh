#include "Endpoint.h"
#include <cerrno>
#include <exception>
#include <ace/common/Log.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

namespace mesh {

Endpoint::Endpoint(config::IEndpoint const & cfg)
  : m_fd(-1)
  , m_shutdown(false)
  , m_thread()
  , m_clients()
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
  ACE_LOG(Info, "Endpoint created, listening on port: ", cfg.port());
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
  ::close(m_fd);
  ACE_LOG(Info, "Endpoint destructed");
}

#define BUFFER_SIZE 16384

void
Endpoint::run()
{
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  uint8_t * buffer = new uint8_t[BUFFER_SIZE];
  //
  // Prepare the poll descriptors.
  //
  struct pollfd fds[1 + m_clients.size()];
  memset(fds, 0, sizeof(fds));
  //
  // Runner loop.
  //
  ACE_LOG(Info, "Endpoint runner started");
  while (!m_shutdown) {
    //
    // Reset the poll descriptors.
    //
    fds[0].fd = m_fd;
    fds[0].events = POLLIN;
    for (size_t i = 0; i < m_clients.size(); i += 1) {
      fds[i + 1].fd = m_clients[i];
      fds[i + 1].events = POLLIN;
    }
    //
    // Wait for a connection.
    //
    int res = poll(fds, 1 + m_clients.size(), 100);
    if (res <= 0) {
      continue;
    }
    //
    // Process the server socket.
    //
    if (fds[0].revents & POLLIN) {
      ACE_LOG(Info, "Accepting new client connection");
      int fd = accept(m_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
      if (fd >= 0) {
        fcntl(fd, O_NONBLOCK);
        m_clients.push_back(fd);
      }
    }
    //
    // Process the client sockets.
    //
    for (size_t i = 0; i < m_clients.size(); i += 1) {
      if (fds[i + 1].revents & POLLIN) {
        read(fds[i + 1].fd, buffer, BUFFER_SIZE);
      }
    }
  }
  ACE_LOG(Info, "Endpoint runner ended");
  delete[] buffer;
}

}
