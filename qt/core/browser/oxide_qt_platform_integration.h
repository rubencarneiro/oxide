// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_BROWSER_PLATFORM_INTEGRATION_H_
#define _OXIDE_QT_CORE_BROWSER_PLATFORM_INTEGRATION_H_

#include "base/memory/ref_counted.h"

#include "shared/browser/oxide_platform_integration.h"

namespace oxide {
namespace qt {

class GLContext;

class PlatformIntegration final : public oxide::PlatformIntegration {
 public:
  PlatformIntegration();
  ~PlatformIntegration();

 private:
  bool LaunchURLExternally(const GURL& url) final;
  bool IsTouchSupported() final;
  intptr_t GetNativeDisplay() final;
  blink::WebScreenInfo GetDefaultScreenInfo() final;
  oxide::GLContextAdopted* GetGLShareContext() final;

  scoped_refptr<GLContextAdopted> gl_share_context_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_PLATFORM_INTEGRATION_H_
