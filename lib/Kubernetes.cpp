#include "Kubernetes.h"
#include <ace/common/Log.h>
#include <ace/engine/Master.h>
#include <ace/tree/Array.h>
#include <ace/tree/Primitive.h>

using namespace ace::tree;

namespace mesh {
namespace k8s {

void
buildService(std::string const & name, int port, Value::Ref const & svc,
             std::list<ace::tree::Value::Ref> & res)
{
  auto root = std::static_pointer_cast<Object>(svc->clone());
  /*
   * Define the paths.
   */
  Path pName = Path::parse("$.metadata.name");
  Path pSlct = Path::parse("$.spec.selector.job-name");
  Path pPort = Path::parse("$.spec.ports[0].port");
  /*
   * Fill the values.
   */
  root->put(pName, Primitive::build("name", name));
  root->put(pSlct, Primitive::build("job-name", name));
  root->put(pPort, Primitive::build("port", port));
  /*
   * Return the object.
   */
  res.push_back(root);
}

void
buildJob(std::string const & name, int port, Value::Ref const & job,
         std::list<ace::tree::Value::Ref> & res)
{
  auto root = std::static_pointer_cast<Object>(job->clone());
  /*
   * Define the paths.
   */
  Path pName = Path::parse("$.metadata.name");
  Path pComd = Path::parse("$.spec.template.spec.containers[0].command");
  Path pPort = Path::parse("$.spec.template.spec.containers[0].ports[0].containerPort");
  /*
   * Fill the values.
   */
  root->put(pName, Primitive::build("name", name));
  root->put(pComd, Primitive::build("/data/" + name + ".json"));
  root->put(pPort, Primitive::build("containerPort", port));
  /*
   * Return the object.
   */
  res.push_back(root);
}
}}
