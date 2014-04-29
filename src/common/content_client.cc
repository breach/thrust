// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/common/content_client.h"

#include "base/command_line.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/content_switches.h"
#include "exo_browser/src/common/switches.h"
#include "grit/exo_browser_resources.h"
#include "grit/webkit_resources.h"
#include "grit/webkit_strings.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "webkit/common/user_agent/user_agent_util.h"

namespace exo_browser {

ExoBrowserContentClient::~ExoBrowserContentClient() 
{
}

std::string 
ExoBrowserContentClient::GetUserAgent() const 
{
  std::string product = "Chrome/" UA_VERSION;
  return webkit_glue::BuildUserAgentFromProduct(product);
}

base::string16 
ExoBrowserContentClient::GetLocalizedString(
    int message_id) const 
{
  return l10n_util::GetStringUTF16(message_id);
}

base::StringPiece 
ExoBrowserContentClient::GetDataResource(
    int resource_id,
    ui::ScaleFactor scale_factor) const 
{
  return ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(
      resource_id, scale_factor);
}

base::RefCountedStaticMemory* 
ExoBrowserContentClient::GetDataResourceBytes(
    int resource_id) const 
{
  return ResourceBundle::GetSharedInstance().LoadDataResourceBytes(resource_id);
}

gfx::Image& 
ExoBrowserContentClient::GetNativeImageNamed(
    int resource_id) const 
{
  return ResourceBundle::GetSharedInstance().GetNativeImageNamed(resource_id);
}

} // namespace exo_browser
