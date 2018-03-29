#include <Connection.h>
#include <Endpoint.h>
#include <Status.h>
#include <Mesh.ac.h>
#include <list>
#include <string>
#include <ace/engine/Master.h>
#include <ace/model/Helper.h>
#include <tclap/CmdLine.h>
#include <signal.h>
#include <unistd.h>

using namespace ace::model::Helper;

template<class T> using VA = TCLAP::ValueArg<T>;

static bool terminated = false;

void
signal_handler(int signum)
{
  terminated = true;
}

int
main(int argc, char *argv[])
{
  /*
   * Define CLI arguments
   */
  TCLAP::CmdLine cmd("Node", ' ', MESH_VERSION);
  VA<int>         bckA("b", "backoff", "Back-off delay (us)", false, 1000000, "uS", cmd);
  VA<std::string> cfgA("c", "config", "Configuration file", true, "", "CONFIG", cmd);
  cmd.parse(argc, argv);
  /*
   * Instantiate the configuration
   */
  auto cfg = parseFile<mesh::config::Mesh>(cfgA.getValue(), false, argc, argv);
  if (cfg == nullptr) return __LINE__;
  /*
   * Create the status file.
   */
  mesh::Status status("/tmp/node");
  /*
   * Register the TERM signal.
   */
  signal(SIGTERM, signal_handler);
  /*
   * Create the endpoint.
   */
  ACE_LOG(Info, "Creating the endpoint");
  mesh::Endpoint ep(*cfg->endpoint());
  /*
   * Create the connections.
   */
  std::list<mesh::Connection *> connections;
  if (cfg->has_connections()) {
    ACE_LOG(Info, "Creating the connections");
    for (auto const & c: cfg->connections()) {
      bool keep_trying = true;
      mesh::Connection * v = nullptr;
      /*
       * Try connecting the endpoint.
       */
      while (keep_trying) try {
        v = new mesh::Connection(*c);
        keep_trying = false;
      }
      catch (std::runtime_error const &) {
        ACE_LOG(Info, "Connection refused, retrying in 1 second");
        usleep(bckA.getValue());
      }
      /*
       * Queue that connection.
       */
      connections.push_back(v);
    }
  }
  /*
   * Create the status file and return.
   */
  status.mark();
  /*
   * Wait for the termination signal.
   */
  while (!terminated) {
    usleep(100000);
  }
  return 0;
}
