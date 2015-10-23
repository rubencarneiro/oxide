// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "oxide_constants.h"

namespace switches {

const char kEnableGoogleTalkPlugin[] = "enable-google-talk-plugin";
const char kPepperFlashPluginPath[] = "pepper-flash-plugin-path";
const char kFormFactor[] = "form-factor";
const char kFormFactorDesktop[] = "desktop";
const char kFormFactorTablet[] = "tablet";
const char kFormFactorPhone[] = "phone";
const char kIncognito[] = "incognito";
const char kEnableMediaHubAudio[] = "enable-media-hub-audio";
const char kMediaHubFixedSessionDomains[] = "media-hub-fixed-session-domains";

} // namespace switches

namespace oxide {

const int kMainWorldId = 0;
const char kMainWorldContextUrl[] = "oxide://main-world";
const char kImageContextMenuPropertiesMimeType[] = "mimeType";

} // namespace oxide
