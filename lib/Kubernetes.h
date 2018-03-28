#pragma once

#include <ace/tree/Value.h>
#include <list>
#include <string>

namespace mesh {
namespace k8s {

void
buildJob(std::string const & name, const int port,
         ace::tree::Value::Ref const & job,
         std::list<ace::tree::Value::Ref> & res);

void
buildService(std::string const & name, const int port,
             ace::tree::Value::Ref const & svc,
             std::list<ace::tree::Value::Ref> & res);



}}
