// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "power_save_blocker_unity8.h"

#include <memory>

#include "base/bind.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "content/public/browser/browser_thread.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_path.h"
#include "dbus/object_proxy.h"

#include "shared/browser/oxide_browser_platform_integration.h"

namespace oxide {

namespace {
const int32_t kInvalidCookie = -1;

const char kUnityScreenServiceName[] = "com.canonical.Unity.Screen";
const char kUnityScreenPath[] = "/com/canonical/Unity/Screen";
const char kUnityScreenInterface[] = "com.canonical.Unity.Screen";
}

class PowerSaveBlockerUnity8::Core : public base::RefCountedThreadSafe<Core> {
 public:
  Core()
      : cookie_(kInvalidCookie) {}

  void ApplyBlock();
  void RemoveBlock();

 private:
  friend class base::RefCountedThreadSafe<Core>;
  ~Core();

  void ApplyBlockOnFileThread();
  void RemoveBlockOnFileThread();

  scoped_refptr<dbus::Bus> bus_;

  int32_t cookie_;

  DISALLOW_COPY_AND_ASSIGN(Core);
};

PowerSaveBlockerUnity8::Core::~Core() {
  DCHECK_EQ(cookie_, kInvalidCookie);
  DCHECK(!bus_);
}

void PowerSaveBlockerUnity8::Core::ApplyBlockOnFileThread() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::FILE);

  if (cookie_ != kInvalidCookie) {
    return;
  }

  DCHECK(!bus_.get());
  dbus::Bus::Options options;
  options.bus_type = dbus::Bus::SYSTEM;
  options.connection_type = dbus::Bus::PRIVATE;
  bus_ = new dbus::Bus(options);

  scoped_refptr<dbus::ObjectProxy> object_proxy =
      bus_->GetObjectProxy(kUnityScreenServiceName,
                           dbus::ObjectPath(kUnityScreenPath));
  std::unique_ptr<dbus::MethodCall> method_call =
      base::MakeUnique<dbus::MethodCall>(kUnityScreenInterface,
                                         "keepDisplayOn");

  dbus::MessageWriter message_writer(method_call.get());

  std::unique_ptr<dbus::Response> response =
      object_proxy->CallMethodAndBlock(method_call.get(),
                                       dbus::ObjectProxy::TIMEOUT_USE_DEFAULT);
  if (response) {
    dbus::MessageReader message_reader(response.get());
    if (!message_reader.PopInt32(&cookie_)) {
      LOG(ERROR) << "Invalid response for screen blanking inhibition request: "
                 << response->ToString();
    }
  } else {
    LOG(ERROR) << "Failed to inhibit screen blanking";
  }
}

void PowerSaveBlockerUnity8::Core::RemoveBlockOnFileThread() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::FILE);

  if (cookie_ != kInvalidCookie) {
    DCHECK(bus_.get());
    scoped_refptr<dbus::ObjectProxy> object_proxy =
        bus_->GetObjectProxy(kUnityScreenServiceName,
                             dbus::ObjectPath(kUnityScreenPath));
    std::unique_ptr<dbus::MethodCall> method_call =
        base::MakeUnique<dbus::MethodCall>(kUnityScreenInterface,
                                           "removeDisplayOnRequest");

    dbus::MessageWriter message_writer(method_call.get());
    message_writer.AppendInt32(cookie_);

    object_proxy->CallMethodAndBlock(method_call.get(),
                                     dbus::ObjectProxy::TIMEOUT_USE_DEFAULT);
    cookie_ = kInvalidCookie;
  }

  if (bus_.get()) {
    bus_->ShutdownAndBlock();
    bus_ = nullptr;
  }
}

void PowerSaveBlockerUnity8::Core::ApplyBlock() {
  content::BrowserThread::PostTask(
      content::BrowserThread::FILE,
      FROM_HERE,
      base::Bind(&Core::ApplyBlockOnFileThread, this));
}

void PowerSaveBlockerUnity8::Core::RemoveBlock() {
  content::BrowserThread::PostTask(
      content::BrowserThread::FILE,
      FROM_HERE,
      base::Bind(&Core::RemoveBlockOnFileThread, this));
}

void PowerSaveBlockerUnity8::ApplicationStateChanged() {
  BrowserPlatformIntegration::ApplicationState state =
      BrowserPlatformIntegration::GetInstance()->GetApplicationState();
  if (state == BrowserPlatformIntegration::APPLICATION_STATE_SUSPENDED) {
    core_->RemoveBlock();
  } else {
    core_->ApplyBlock();
  }
}

PowerSaveBlockerUnity8::PowerSaveBlockerUnity8()
    : core_(new Core()) {
  if (BrowserPlatformIntegration::GetInstance()->GetApplicationState() !=
      BrowserPlatformIntegration::APPLICATION_STATE_SUSPENDED) {
    core_->ApplyBlock();
  }
}

PowerSaveBlockerUnity8::~PowerSaveBlockerUnity8() {
  core_->RemoveBlock();
}

} // namespace oxide
