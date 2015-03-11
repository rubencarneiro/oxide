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

#include "oxide_browser_platform_integration.h"

#include "base/logging.h"

#include "oxide_browser_platform_integration_observer.h"

namespace oxide {

namespace {

BrowserPlatformIntegration* g_instance;

}

BrowserPlatformIntegration::BrowserPlatformIntegration() {
  CHECK(!g_instance)
      << "Can't create more than one BrowserPlatformIntegration instance";
  g_instance = this;
}

BrowserPlatformIntegration::~BrowserPlatformIntegration() {
  DCHECK_EQ(g_instance, this);
  g_instance = nullptr;
}

// static
BrowserPlatformIntegration* BrowserPlatformIntegration::GetInstance() {
  DCHECK(g_instance);
  return g_instance;
}

bool BrowserPlatformIntegration::LaunchURLExternally(const GURL& url) {
  return false;
}

bool BrowserPlatformIntegration::IsTouchSupported() {
  return false;
}

GLContextDependent* BrowserPlatformIntegration::GetGLShareContext() {
  return nullptr;
}

void BrowserPlatformIntegration::BrowserThreadInit(
    content::BrowserThread::ID id) {}

void BrowserPlatformIntegration::BrowserThreadCleanUp(
    content::BrowserThread::ID id) {}

content::LocationProvider*
BrowserPlatformIntegration::CreateLocationProvider() {
  return nullptr;
}

BrowserPlatformIntegration::ApplicationState
BrowserPlatformIntegration::GetApplicationState() {
  return APPLICATION_STATE_ACTIVE;
}

void BrowserPlatformIntegration::AddObserver(
    BrowserPlatformIntegrationObserver* observer) {
  observers_.AddObserver(observer);
}

void BrowserPlatformIntegration::RemoveObserver(
    BrowserPlatformIntegrationObserver* observer) {
  observers_.RemoveObserver(observer);
}

void BrowserPlatformIntegration::NotifyApplicationStateChanged() {
  FOR_EACH_OBSERVER(BrowserPlatformIntegrationObserver,
                    observers_,
                    ApplicationStateChanged());
}

} // namespace oxide
