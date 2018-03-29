#pragma once

#include <atomic>
#include <thread>
#include <vector>

namespace mesh {

class Monitor
{
 public:

  Monitor(const int ms);
  ~Monitor();

 private:

  void run();

  int               m_fd;
  std::atomic<bool> m_shutdown;
  std::thread       m_thread;
};

}
