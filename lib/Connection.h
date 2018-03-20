#pragma once

#include "IConnection.ac.h"

namespace mesh {

class Connection
{
 public:

  Connection(config::IConnection const & cfg);
  ~Connection();

 private:

  int m_fd;
};

}
