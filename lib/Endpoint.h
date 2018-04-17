#pragma once

#include "IEndpoint.ac.h"
#include <atomic>
#include <thread>
#include <vector>

namespace mesh {

class Endpoint
{
 public:

  Endpoint(config::IEndpoint const & cfg, const int dur, const int sec);
  ~Endpoint();

  inline bool done()
  {
    return m_done;
  }

 private:

  void run();

  int               m_duration;
  int               m_sec;
  int               m_fd;
  int               m_timerfd;
  std::atomic<bool> m_shutdown;
  std::thread       m_thread;
  std::vector<int>  m_clients;
  size_t            m_expected;
  std::atomic<bool> m_done;
};

}
