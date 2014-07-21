// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/browser/util/renderer_preferences_util.h"

#include "base/prefs/pref_service.h"
#include "content/public/common/renderer_preferences.h"
#include "third_party/skia/include/core/SkColor.h"

#if defined(OS_LINUX) || defined(OS_ANDROID)
#include "ui/gfx/font_render_params_linux.h"
#endif

#if defined(TOOLKIT_GTK)
#include "ui/gfx/gtk_util.h"
#endif

#if defined(TOOLKIT_VIEWS)
#include "ui/views/controls/textfield/textfield.h"
#endif

#if defined(USE_AURA) && defined(OS_LINUX) && !defined(OS_CHROMEOS)
#include "ui/views/linux_ui/linux_ui.h"
#endif

namespace renderer_preferences_util {

namespace {

#if defined(TOOLKIT_GTK)
// Dividing GTK's cursor blink cycle time (in milliseconds) by this value yields
// an appropriate value for content::RendererPreferences::caret_blink_interval.
// This matches the logic in the WebKit GTK port.
const double kGtkCursorBlinkCycleFactor = 2000.0;
#endif  // defined(TOOLKIT_GTK)

#if defined(OS_LINUX) || defined(OS_ANDROID)
content::RendererPreferencesHintingEnum 
GetRendererPreferencesHintingEnum(
    gfx::FontRenderParams::Hinting hinting) 
{
  switch (hinting) {
    case gfx::FontRenderParams::HINTING_NONE:
      return content::RENDERER_PREFERENCES_HINTING_NONE;
    case gfx::FontRenderParams::HINTING_SLIGHT:
      return content::RENDERER_PREFERENCES_HINTING_SLIGHT;
    case gfx::FontRenderParams::HINTING_MEDIUM:
      return content::RENDERER_PREFERENCES_HINTING_MEDIUM;
    case gfx::FontRenderParams::HINTING_FULL:
      return content::RENDERER_PREFERENCES_HINTING_FULL;
    default:
      NOTREACHED() << "Unhandled hinting style " << hinting;
      return content::RENDERER_PREFERENCES_HINTING_SYSTEM_DEFAULT;
  }
}

content::RendererPreferencesSubpixelRenderingEnum
GetRendererPreferencesSubpixelRenderingEnum(
    gfx::FontRenderParams::SubpixelRendering subpixel_rendering) 
{
  switch (subpixel_rendering) {
    case gfx::FontRenderParams::SUBPIXEL_RENDERING_NONE:
      return content::RENDERER_PREFERENCES_SUBPIXEL_RENDERING_NONE;
    case gfx::FontRenderParams::SUBPIXEL_RENDERING_RGB:
      return content::RENDERER_PREFERENCES_SUBPIXEL_RENDERING_RGB;
    case gfx::FontRenderParams::SUBPIXEL_RENDERING_BGR:
      return content::RENDERER_PREFERENCES_SUBPIXEL_RENDERING_BGR;
    case gfx::FontRenderParams::SUBPIXEL_RENDERING_VRGB:
      return content::RENDERER_PREFERENCES_SUBPIXEL_RENDERING_VRGB;
    case gfx::FontRenderParams::SUBPIXEL_RENDERING_VBGR:
      return content::RENDERER_PREFERENCES_SUBPIXEL_RENDERING_VBGR;
    default:
      NOTREACHED() << "Unhandled subpixel rendering style "
                   << subpixel_rendering;
      return content::RENDERER_PREFERENCES_SUBPIXEL_RENDERING_SYSTEM_DEFAULT;
  }
}
#endif  // defined(OS_LINUX) || defined(OS_ANDROID)

}  // namespace

void 
UpdateFromSystemSettings(
    content::RendererPreferences* prefs) 
{
#if defined(TOOLKIT_GTK)
  // Dividing GTK's cursor blink cycle time (in milliseconds) by this value
  // yields an appropriate value for RendererPreferences::caret_blink_interval.
  // This matches the logic in the WebKit GTK port.
  const double kGtkCursorBlinkCycleFactor = 2000.0;
  const base::TimeDelta cursor_blink_time = gfx::GetCursorBlinkCycle();
  prefs->caret_blink_interval =
      cursor_blink_time.InMilliseconds() ?
      cursor_blink_time.InMilliseconds() / kGtkCursorBlinkCycleFactor :
      0;
#elif defined(USE_DEFAULT_RENDER_THEME)
  prefs->focus_ring_color = SkColorSetRGB(0x4D, 0x90, 0xFE);

#if defined(OS_CHROMEOS)
  // This color is 0x544d90fe modulated with 0xffffff.
  prefs->active_selection_bg_color = SkColorSetRGB(0xCB, 0xE4, 0xFA);
  prefs->active_selection_fg_color = SK_ColorBLACK;
  prefs->inactive_selection_bg_color = SkColorSetRGB(0xEA, 0xEA, 0xEA);
  prefs->inactive_selection_fg_color = SK_ColorBLACK;
#endif

#endif

#if defined(TOOLKIT_VIEWS)
  prefs->caret_blink_interval = views::Textfield::GetCaretBlinkMs() / 1000.0;
#endif

#if defined(USE_AURA) && defined(OS_LINUX) && !defined(OS_CHROMEOS)
  views::LinuxUI* linux_ui = views::LinuxUI::instance();
  if (linux_ui) {
    // If we have a linux_ui object, set the caret blink interval regardless of
    // whether we're in native theme mode.
    prefs->caret_blink_interval = linux_ui->GetCursorBlinkInterval();
  }
#endif

#if defined(OS_LINUX) || defined(OS_ANDROID)
  const gfx::FontRenderParams& params = gfx::GetDefaultWebKitFontRenderParams();
  prefs->should_antialias_text = params.antialiasing;
  prefs->use_subpixel_positioning = params.subpixel_positioning;
  prefs->hinting = GetRendererPreferencesHintingEnum(params.hinting);
  prefs->use_autohinter = params.autohinter;
  prefs->use_bitmaps = params.use_bitmaps;
  prefs->subpixel_rendering =
      GetRendererPreferencesSubpixelRenderingEnum(params.subpixel_rendering);
#endif

#if !defined(OS_MACOSX)
  prefs->plugin_fullscreen_allowed = true;
#endif
}

} // namespace renderer_preferences_util

