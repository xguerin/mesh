#pragma once

#include "IEndpoint.ac.h"
#include <atomic>
#include <list>
#include <thread>

namespace mesh {

class Endpoint
{
 public:

  Endpoint(config::IEndpoint const & cfg);
  ~Endpoint();

 private:

  void run();

  int               m_fd;
  std::atomic<bool> m_shutdown;
  std::thread       m_thread;
  std::list<int>    m_clients;
  size_t            m_countdown;
};

}
