#pragma once

#include "IEndpoint.ac.h"
#include <atomic>
#include <thread>
#include <vector>

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
  std::vector<int>  m_clients;
};

}
