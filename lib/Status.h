#pragma once

#include <fstream>
#include <string>

namespace mesh {

class Status
{
 public:

  Status(std::string const & fp);
  ~Status();

  void mark();

 private:

  std::string   m_fp;
  std::ofstream m_file;
};

}
