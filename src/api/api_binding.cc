// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include <sstream>
#include <string>

#include "src/api/exo_shell_binding.h"

namespace exo_shell {

ApiBinding::ApiBinding(
    const std::string& type,
    const unsigned int id)
  : type_(type),
    id_(id)
{
}

ApiBinding::~ApiBinding()
{
}

void
ApiBinding::RemoteCall(
    const std::string& method,
    scoped_ptr<base::DictionaryValue> args,
    const ApiHandler::ActionCallback& callback)
{
  /* TODO(spolu) */
}

void
ApiBinding::Emit(
    const std::string& type,
    scoped_ptr<base::DictionaryValue> event)
{
  /* TODO(spolu) */
}
  
void
ApiBinding::RegisterEventForCollection(unsigned int id)
{
  event_ids.push_back(id);
}

std::vector<unsigned int>&
ApiBinding::GetRegisteredEvents()
{
  return event_ids;
}

} // namespace exo_shell
