#include "Monitor.h"
#include <cerrno>
#include <exception>
#include <ace/common/Log.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

namespace mesh {

Monitor::Monitor(const int ms)
  : m_fd(-1)
  , m_shutdown(false)
  , m_thread()
{

}

Monitor::~Monitor()
{

}

void
Monitor::run()
{

}

}
