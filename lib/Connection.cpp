#include "Connection.h"
#include <cerrno>
#include <ace/common/Log.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

namespace mesh {

Connection::Connection(config::IConnection const & cfg)
  : m_fd(-1)
{
  //
  // Create the client socket.
  //
  if ((m_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    ACE_LOG(Debug, strerror(errno));
    throw std::runtime_error(strerror(errno));
  }
  //
  // Get address info.
  //
  struct hostent * hosts;
  hosts = gethostbyname(cfg.hostname().c_str());
  if (hosts == nullptr) {
    ACE_LOG(Debug, strerror(errno));
    throw std::runtime_error(strerror(errno));
  }
  //
  // Fill-in the server address.
  //
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(cfg.port());
  bcopy(hosts->h_addr, &addr.sin_addr.s_addr, hosts->h_length);
  ACE_LOG(Info, "Connecting to ", cfg.hostname(), " = ", inet_ntoa(addr.sin_addr));
  //
  // Connect to the remove server.
  //
  if (connect(m_fd, (struct sockaddr *)&addr, sizeof(sockaddr_in)) < 0) {
    ACE_LOG(Debug, strerror(errno));
    throw std::runtime_error(strerror(errno));
  }
}

Connection::~Connection()
{
  ::close(m_fd);
}

ssize_t
Connection::write(const size_t n, const void * const data)
{
  return ::write(m_fd, data, n);
}

}
