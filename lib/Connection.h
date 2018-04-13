#pragma once

#include "IConnection.ac.h"

namespace mesh {

class Connection
{
 public:

  Connection(config::IConnection const & cfg);
  ~Connection();

  ssize_t write(const size_t n, const void * const data);

 private:

  int m_fd;
};

}
