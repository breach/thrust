// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_UTIL_RENDERER_PREFERENCES_UTIL_H_
#define EXO_BROWSER_BROWSER_UTIL_RENDERER_PREFERENCES_UTIL_H_

namespace content {
struct RendererPreferences;
}

namespace renderer_preferences_util {

// Copies system configuration preferences into |prefs|.
void UpdateFromSystemSettings(content::RendererPreferences* prefs);

} // namespace renderer_preferences_util

#endif // EXO_BROWSER_BROWSER_UTIL_RENDERER_PREFERENCES_UTIL_H_
