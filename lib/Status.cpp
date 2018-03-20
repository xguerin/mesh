#include "Status.h"
#include <unistd.h>

namespace mesh {

Status::Status(std::string const & fp)
  : m_fp(fp)
  , m_file()
{
  ::unlink(m_fp.c_str());
}

Status::~Status()
{
  m_file.close();
}

void
Status::mark()
{
  m_file.open(m_fp);
}

}
