// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/browser_features.h"

namespace features {

#if defined(OS_CHROMEOS)
// Enables being able to zoom a web page by double tapping in Chrome OS tablet
// mode.
const base::Feature kDoubleTapToZoomInTabletMode{
    "DoubleTapToZoomInTabletMode", base::FEATURE_DISABLED_BY_DEFAULT};
#endif

#if !defined(OS_ANDROID)
// Whether to enable "dark mode" enhancements in Mac Mojave or Windows 10 for
// UIs implemented with web technologies.
const base::Feature kWebUIDarkMode{"WebUIDarkMode",
                                   base::FEATURE_DISABLED_BY_DEFAULT};
#endif

// Enables popup cards containing tab information when hovering over a tab.
const base::Feature kTabHoverCards{"TabHoverCards",
                                   base::FEATURE_DISABLED_BY_DEFAULT};

}  // namespace features
