// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/common/content_client.h"

#include <string>
#include <vector>

#include "src/common/chrome_version.h"

namespace exo_shell {

ContentClient::ContentClient() 
{
}

ContentClient::~ContentClient() 
{
}

std::string 
ContentClient::GetProduct() const 
{
  return "Chrome/" CHROME_VERSION_STRING;
}

void 
ContentClient::AddAdditionalSchemes(
    std::vector<std::string>* standard_schemes,
    std::vector<std::string>* savable_schemes) 
{
  standard_schemes->push_back("chrome-extension");
}

}  // namespace exo_shell
