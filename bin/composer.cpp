#include <Graph.ac.h>
#include <Kubernetes.h>
#include <string>
#include <fstream>
#include <ace/engine/Master.h>
#include <ace/model/Helper.h>
#include <tclap/CmdLine.h>
#include <unistd.h>

using namespace ace::model::Helper;
using namespace ace::tree;

template<class T> using VA = TCLAP::ValueArg<T>;
template<class T> using UA = TCLAP::UnlabeledValueArg<T>;

void generateKubeConfiguration(mesh::config::IGraph const & graph,
                               Value::Ref const & svc, Value::Ref const & job)
{
  std::list<Value::Ref> elems;
  for (auto const & n: graph.nodes()) {
    mesh::k8s::buildService(n.first, n.second->port(), svc, elems);
  }
  for (auto const & n: graph.nodes()) {
    mesh::k8s::buildJob(n.first, n.second->port(), job, elems);
  }
  /*
   * Generate the YAML file.
   */
  std::string fn("mesh.yaml");
  std::ofstream ofs(fn);
  MASTER.scannerByExtension(fn).dumpAll(elems, Scanner::Format::Default, ofs);
  ofs.close();
}

void
generateNodeConfiguration(mesh::config::IGraph const & graph)
{
  for (auto const & n: graph.nodes()) {
    size_t clients = 0;
    auto root = Object::build();
    /*
     * Get the number of incoming connections.
     */
    for (auto const & c: graph.connections()) {
      if (c.first == n.first) {
        continue;
      }
      for (auto const & i: c.second) {
        if (i == n.first) {
          clients += 1;
        }
      }
    }
    /*
     * Create the endpoint definition.
     */
    auto ep = Object::build("endpoint");
    ep->put(Primitive::build("clients", clients));
    ep->put(Primitive::build("port", n.second->port()));
    root->put(ep);
    /*
     * Create the connection definitions.
     */
    size_t index = 0;
    auto cl = Array::build("connections");
    for (auto const & c: graph.connections().at(n.first)) {
      if (graph.nodes().count(c) == 0) {
        ACE_LOG(Error, "No such node: ", c);
        continue;
      }
      auto tg = graph.nodes().at(c);
      auto cd = Object::build(std::to_string(index));
      cd->put(Primitive::build("hostname", tg->hostname()));
      cd->put(Primitive::build("port", tg->port()));
      cl->push_back(cd);
    }
    root->put(cl);
    /*
     * Dump the configuration.
     */
    std::string fn(n.first + ".json");
    std::ofstream ofs(fn);
    MASTER.scannerByExtension(fn).dump(*root, Scanner::Format::Default, ofs);
    ofs.close();
  }
}

int
main(int argc, char *argv[]) try
{
  /*
   * Define CLI arguments
   */
  TCLAP::CmdLine cmd("Composer", ' ', MESH_VERSION);
  VA<std::string> svcA("s", "service", "Service template", true, "", "YAML", cmd);
  VA<std::string> jobA("j", "job", "Job template", true, "", "YAML", cmd);
  UA<std::string> cfgA("config", "Configuration file", true, "", "CONFIG", cmd);
  cmd.parse(argc, argv);
  /*
   * Instantiate the configuration
   */
  auto cfg = parseFile<mesh::config::Graph>(cfgA.getValue(), false, argc, argv);
  if (cfg == nullptr) return __LINE__;
  /*
   * Open the templates.
   */
  auto svc = MASTER.scannerByName("yaml").open(svcA.getValue(), 0, NULL);
  if (svc == nullptr) return __LINE__;
  auto job = MASTER.scannerByName("yaml").open(jobA.getValue(), 0, NULL);
  if (job == nullptr) return __LINE__;
  /*
   * Generate the node files.
   */
  generateNodeConfiguration(*cfg);
  generateKubeConfiguration(*cfg, svc, job);
  return 0;
}
catch (std::runtime_error const & e)
{
  ACE_LOG(Error, "Exception: ", e.what());
}
