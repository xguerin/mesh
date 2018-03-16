#include "Connection.h"
#include "Endpoint.h"
#include <Mesh.ac.h>
#include <list>
#include <string>
#include <ace/engine/Master.h>
#include <ace/model/Helper.h>
#include <tclap/CmdLine.h>
#include <unistd.h>

using namespace ace::model::Helper;

template<class T>
using VA = TCLAP::ValueArg<T>;

int main(int argc, char *argv[]) try
{
  //
  // Define CLI arguments
  //
  TCLAP::CmdLine cmd("Mesh", ' ', MESH_VERSION);
  VA<std::string> cfgA("c", "config", "Configuration file", true, "", "CONFIG", cmd);
  cmd.parse(argc, argv);
  //
  // Instantiate the configuration
  //
  auto cfg = parseFile<mesh::config::Mesh>(cfgA.getValue(), false, argc, argv);
  if (cfg == nullptr) return __LINE__;
  //
  // Create the endpoint.
  //
  ACE_LOG(Info, "Creating the endpoint");
  mesh::Endpoint ep(*cfg->endpoint());
  //
  // Create the connections.
  //
  std::list<mesh::Connection *> connections;
  if (cfg->has_connections()) {
    ACE_LOG(Info, "Creating the connections");
    for (auto const & c: cfg->connections()) {
      connections.push_back(new mesh::Connection(*c));
    }
  }
  return 0;
}
catch (std::runtime_error const &)
{

}
