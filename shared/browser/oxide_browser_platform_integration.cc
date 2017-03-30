// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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
#include "device/geolocation/location_provider.h"

#include "shared/browser/clipboard/oxide_clipboard_dummy_impl.h"

#include "oxide_browser_platform_integration_observer.h"
#include "oxide_drag_source.h"

namespace oxide {

namespace {

const char kDefaultApplicationName[] = "Oxide";

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

GLContextDependent* BrowserPlatformIntegration::GetGLShareContext() {
  return nullptr;
}

Clipboard* BrowserPlatformIntegration::CreateClipboard() {
  return new ClipboardDummyImpl();
}

void BrowserPlatformIntegration::BrowserThreadInit(
    content::BrowserThread::ID id) {}

void BrowserPlatformIntegration::BrowserThreadCleanUp(
    content::BrowserThread::ID id) {}

std::unique_ptr<device::LocationProvider>
BrowserPlatformIntegration::CreateLocationProvider() {
  return nullptr;
}

BrowserPlatformIntegration::ApplicationState
BrowserPlatformIntegration::GetApplicationState() {
  return APPLICATION_STATE_ACTIVE;
}

std::string BrowserPlatformIntegration::GetApplicationName() {
  return kDefaultApplicationName;
}

std::unique_ptr<DragSource> BrowserPlatformIntegration::CreateDragSource(
    DragSourceClient* client) {
  return nullptr;
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
  for (auto& observer : observers_) {
    observer.ApplicationStateChanged();
  }
}

void BrowserPlatformIntegration::CreateVibrationManager(
      mojo::InterfaceRequest<device::mojom::VibrationManager> request) {}

} // namespace oxide
