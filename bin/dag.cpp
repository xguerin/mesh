#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <ace/common/Log.h>
#include <ace/engine/Master.h>
#include <ace/tree/Array.h>
#include <ace/tree/Object.h>
#include <ace/tree/Primitive.h>
#include <tclap/CmdLine.h>

/*
 * Orignal code: https://gist.github.com/submachine/3855851
 */

template<class T> using VA = TCLAP::ValueArg<T>;

using Nodes = std::list<int>;
using Edges = std::map<int, Nodes>;

using namespace ace::tree;

void
makeGraphFile(const int node_count, Edges const & edges, std::string const & o)
{
  auto root = Object::build();
  /*
   * Generate node configurations.
   */
  auto nodes = Object::build("nodes");
  for (int i = 0; i < node_count; i += 1) {
    std::ostringstream oss;
    oss << "n" << std::setfill('0') << std::setw(3) << std::to_string(i);
    auto node = Object::build(oss.str());
    node->put(Primitive::build("hostname", oss.str() + ".default.svc.cluster.local"));
    node->put(Primitive::build("port", 11000));
    nodes->put(node);
  }
  /*
   * Generate edges configurations.
   */
  auto conns = Object::build("connections");
  for (auto const & c: edges) {
    std::ostringstream oss;
    oss << "n" << std::setfill('0') << std::setw(3) << std::to_string(c.first);
    auto clist = Array::build(oss.str());
    for (auto const & i: c.second) {
      std::ostringstream oss;
      oss << "n" << std::setfill('0') << std::setw(3) << std::to_string(i);
      clist->push_back(Primitive::build(oss.str()));
    }
    conns->put(clist);
  }
  /*
   * Generate empty connections.
   */
  for (int i = 0; i < node_count; i += 1) {
    if (edges.count(i) == 0) {
      std::ostringstream oss;
      oss << "n" << std::setfill('0') << std::setw(3) << std::to_string(i);
      auto clist = Array::build(oss.str());
      conns->put(clist);
    }
  }
  /*
   * Add objects to root.
   */
  root->put(nodes);
  root->put(conns);
  /*
   * Generate the output file.
   */
  std::ofstream ofs(o);
  if (!MASTER.hasScannerByExtension(o)) {
    ACE_LOG(Error, o, ": file format not supported");
    return;
  }
  MASTER.scannerByExtension(o).dump(*root, Scanner::Format::Default, ofs);
  ofs.close();
}

void
makeDotFile(Edges const & edges, std::string const & o)
{
  std::ofstream ofs(o);
  ofs << "digraph {" << std::endl;
  for (auto const & e : edges) {
    for (auto const & i : e.second) {
      ofs << "  " << e.first << " -> " << i << ";" << std::endl;
    }
  }
  ofs << "}" << std::endl;
  ofs.close();
}

int
main(int argc, char ** argv)
{
  /*
   * Define CLI arguments
   */
  TCLAP::CmdLine cmd("DAG Generator", ' ', MESH_VERSION);
  VA<int>         edgA("" , "edge"     , "Likelihood of an edge", false, 30 , "PERCENT"  , cmd);
  VA<int>         mnrA("" , "min-width", "Minimum width"        , false, 1  , "MIN WIDTH", cmd);
  VA<int>         mxrA("" , "max-width", "Maximum width"        , false, 5  , "MAX WIDTH", cmd);
  VA<int>         minA("" , "min-depth", "Minimum depth"        , false, 3  , "MIN DEPTH", cmd);
  VA<int>         maxA("" , "max-depth", "Maximum depth"        , false, 5  , "MAX DEPTH", cmd);
  VA<std::string> outA("o", "output"   , "Output file"          , true , "" , "OUTPUT"   , cmd);
  VA<std::string> dotA("d", "dot"      , "Dot file"             , false, "" , "DOTFILE"  , cmd);
  cmd.parse(argc, argv);
  /*
   * Intitialize RAND.
   */
  int i, j, k, acc = 0;
  srand(time(NULL));
  /*
   * Compute the number of ranks.
   */
  int ranks = minA.getValue() + (rand() % (maxA.getValue() - minA.getValue() + 1));
  /*
   * Generate the digraph.
   */
  Edges edges;
  for (i = 0; i < ranks; i++) {
    /*
     * New nodes of 'higher' rank than all nodes generated till now.
     */
    int new_nodes = mnrA.getValue() + (rand() % (mxrA.getValue() - mnrA.getValue() + 1));
    /*
     * Edges from old nodes ('nodes') to new ones ('new_nodes').
     */
    for (j = 0; j < acc; j++) {
      for (k = acc; k < new_nodes + acc; k++) {
        if ((rand () % 100) < edgA.getValue()) {
          edges[j].push_back(k);
        }}}
    /*
     * Update node count.
     */
    acc += new_nodes;
  }
  /*
   * Close the graph and return.
   */
  makeGraphFile(acc, edges, outA.getValue());
  if (dotA.isSet()) {
    makeDotFile(edges, dotA.getValue());
  }
  return 0;
}
