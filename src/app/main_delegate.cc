// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/app/main_delegate.h"

#include "base/logging.h"
#include "base/command_line.h"
#include "base/debug/stack_trace.h"
#include "content/public/common/content_switches.h"
#include "ui/base/resource/resource_bundle.h"

#include "src/browser/browser_client.h"
#include "src/renderer/renderer_client.h"
#include "src/app/content_client.h"

namespace exo_shell {

MainDelegate::MainDelegate() {
}

MainDelegate::~MainDelegate() {
}

bool 
MainDelegate::BasicStartupComplete(
    int* exit_code) 
{
  // Disable logging out to debug.log on Windows
#if defined(OS_WIN)
  logging::LoggingSettings settings;
#if defined(DEBUG)
  settings.logging_dest = logging::LOG_TO_ALL;
  settings.log_file = L"debug.log";
  settings.lock_log = logging::LOCK_LOG_FILE;
  settings.delete_old = logging::DELETE_OLD_LOG_FILE;
#else
  settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
#endif
  logging::InitLogging(settings);
#endif  // defined(OS_WIN)

  // Logging with pid and timestamp.
  logging::SetLogItems(true, false, true, false);

  // Enable convient stack printing.
#if defined(DEBUG) && defined(OS_LINUX)
  base::debug::EnableInProcessStackDumping();
#endif

  return brightray::MainDelegate::BasicStartupComplete(exit_code);
}

void 
MainDelegate::PreSandboxStartup() 
{
  brightray::MainDelegate::PreSandboxStartup();

  CommandLine* command_line = CommandLine::ForCurrentProcess();
  std::string process_type = command_line->GetSwitchValueASCII(
      switches::kProcessType);

  // Only append arguments for browser process.
  if (!process_type.empty())
    return;

#if defined(OS_WIN)
  // Disable the LegacyRenderWidgetHostHWND, it made frameless windows unable
  // to move and resize. We may consider enabling it again after upgraded to
  // Chrome 38, which should have fixed the problem.
  command_line->AppendSwitch(switches::kDisableLegacyIntermediateWindow);
#endif

  // Disable renderer sandbox for most of node's functions.
  /* TODO(spolu): Make that switch optional! */
  command_line->AppendSwitch(switches::kNoSandbox);
}

content::ContentBrowserClient* 
MainDelegate::CreateContentBrowserClient() {
  browser_client_.reset(new ExoShellBrowserClient);
  return browser_client_.get();
}

content::ContentRendererClient* 
MainDelegate::CreateContentRendererClient() {
  renderer_client_.reset(new ExoShellRendererClient);
  return renderer_client_.get();
}

scoped_ptr<brightray::ContentClient> 
MainDelegate::CreateContentClient() {
  return scoped_ptr<brightray::ContentClient>(new ContentClient).Pass();
}

void 
MainDelegate::AddDataPackFromPath(
    ui::ResourceBundle* bundle, 
    const base::FilePath& pak_dir) 
{
#if defined(OS_WIN)
  bundle->AddDataPackFromPath(
      pak_dir.Append(FILE_PATH_LITERAL("ui_resources_200_percent.pak")),
      ui::SCALE_FACTOR_200P);
  bundle->AddDataPackFromPath(
      pak_dir.Append(FILE_PATH_LITERAL("webkit_resources_200_percent.pak")),
      ui::SCALE_FACTOR_200P);
#endif
}
}
