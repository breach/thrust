// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/common/breach_content_client.h"

#include "base/command_line.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/content_switches.h"
#include "breach/common/breach_switches.h"
#include "grit/shell_resources.h"
#include "grit/webkit_resources.h"
#include "grit/webkit_strings.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "webkit/common/user_agent/user_agent_util.h"

namespace breach {

BreachContentClient::~BreachContentClient() 
{
}

std::string 
BreachContentClient::GetUserAgent() const 
{
  std::string product = "Breach/" BREACH_VERSION;
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  return webkit_glue::BuildUserAgentFromProduct(product);
}

string16 
BreachContentClient::GetLocalizedString(
    int message_id) const 
{
  return l10n_util::GetStringUTF16(message_id);
}

base::StringPiece 
BreachContentClient::GetDataResource(
    int resource_id,
    ui::ScaleFactor scale_factor) const 
{
  return ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(
      resource_id, scale_factor);
}

base::RefCountedStaticMemory* 
BreachContentClient::GetDataResourceBytes(
    int resource_id) const 
{
  return ResourceBundle::GetSharedInstance().LoadDataResourceBytes(resource_id);
}

gfx::Image& 
BreachContentClient::GetNativeImageNamed(
    int resource_id) const 
{
  return ResourceBundle::GetSharedInstance().GetNativeImageNamed(resource_id);
}

}  // namespace content
