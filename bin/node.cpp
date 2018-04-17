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
template<class T> using UA = TCLAP::UnlabeledValueArg<T>;

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
  VA<int>         durA("d", "duration", "Collection duration", false, 30, "S", cmd);
  VA<int>         bckA("b", "backoff", "Back-off delay (us)", false, 1000000, "uS", cmd);
  VA<size_t>      pldA("s", "size", "Payload size", false, 128, "SIZE", cmd);
  VA<int>         perA("p", "period", "Monitor period", false, 1, "SECONDS", cmd);
  UA<std::string> cfgA("config", "Configuration file", true, "", "CONFIG", cmd);
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
  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, signal_handler);
  /*
   * Create the endpoint.
   */
  ACE_LOG(Info, "Creating the endpoint");
  mesh::Endpoint ep(*cfg->endpoint(), durA.getValue(), perA.getValue());
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
        ACE_LOG(Error, "Retrying in 1 second");
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
  if (connections.empty()) {
    while (!terminated && !ep.done()) {
      sleep(1);
    }
  }
  else {
    uint8_t garbage[pldA.getValue()];
    while (!terminated && !ep.done()) {
      for (auto & c: connections) {
        ssize_t res = c->write(pldA.getValue(), garbage);
        if (res < 0) {
          ACE_LOG(Error, "At least one connection lost, terminating");
          terminated = true;
          break;
        }
      }
    }
  }
  return 0;
}
