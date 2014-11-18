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

// Largely inspired by the X11 implementation in
// content/browser/power_save_blocker_x11.cc

#include "oxide_power_save_blocker.h"

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_thread.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_path.h"
#include "dbus/object_proxy.h"

#include "shared/port/content/browser/power_save_blocker_oxide.h"

#include "oxide_browser_platform_integration_observer.h"
#include "oxide_form_factor.h"

namespace oxide {

namespace {

const char kUnityScreenServiceName[] = "com.canonical.Unity.Screen";
const char kUnityScreenPath[] = "/com/canonical/Unity/Screen";
const char kUnityScreenInterface[] = "com.canonical.Unity.Screen";

}

class PowerSaveBlocker : public content::PowerSaveBlockerOxideDelegate,
                         public BrowserPlatformIntegrationObserver {
 public:
  PowerSaveBlocker();

 private:
  virtual ~PowerSaveBlocker() {}

  void Init() final;
  void CleanUp() final;

  void ApplyBlock();
  void RemoveBlock();

  // BrowserPlatformIntegrationObserver implementation
  void ApplicationStateChanged() final;

  oxide::FormFactor form_factor_;
  scoped_refptr<dbus::Bus> bus_;
  int cookie_;
};

void PowerSaveBlocker::Init() {
  if (BrowserPlatformIntegration::GetInstance()->GetApplicationState() ==
      BrowserPlatformIntegration::APPLICATION_STATE_ACTIVE) {
    content::BrowserThread::PostTask(
        content::BrowserThread::FILE,
        FROM_HERE,
        base::Bind(&PowerSaveBlocker::ApplyBlock, this));
  }
}

void PowerSaveBlocker::CleanUp() {
  content::BrowserThread::PostTask(
      content::BrowserThread::FILE,
      FROM_HERE,
      base::Bind(&PowerSaveBlocker::RemoveBlock, this));
}

void PowerSaveBlocker::ApplyBlock() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::FILE));

  if (form_factor_ == oxide::FORM_FACTOR_PHONE ||
      form_factor_ == oxide::FORM_FACTOR_TABLET) {
    DCHECK(!bus_.get());
    dbus::Bus::Options options;
    options.bus_type = dbus::Bus::SYSTEM;
    options.connection_type = dbus::Bus::PRIVATE;
    bus_ = new dbus::Bus(options);

    scoped_refptr<dbus::ObjectProxy> object_proxy = bus_->GetObjectProxy(
          kUnityScreenServiceName,
          dbus::ObjectPath(kUnityScreenPath));
    scoped_ptr<dbus::MethodCall> method_call;
    method_call.reset(
        new dbus::MethodCall(kUnityScreenInterface, "keepDisplayOn"));
    scoped_ptr<dbus::MessageWriter> message_writer;
    message_writer.reset(new dbus::MessageWriter(method_call.get()));

    scoped_ptr<dbus::Response> response(object_proxy->CallMethodAndBlock(
        method_call.get(), dbus::ObjectProxy::TIMEOUT_USE_DEFAULT));
    if (response) {
      dbus::MessageReader message_reader(response.get());
      if (!message_reader.PopInt32(&cookie_)) {
        LOG(ERROR) << "Invalid response for screen blanking inhibition request: "
                   << response->ToString();
      }
    } else {
      LOG(ERROR) << "Failed to inhibit screen blanking";
    }
  } else {
    NOTIMPLEMENTED();
  }
}

void PowerSaveBlocker::RemoveBlock() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::FILE));

  if (form_factor_ == oxide::FORM_FACTOR_PHONE ||
      form_factor_ == oxide::FORM_FACTOR_TABLET) {
    if (cookie_ != 0) {
      DCHECK(bus_.get());
      scoped_refptr<dbus::ObjectProxy> object_proxy = bus_->GetObjectProxy(
          kUnityScreenServiceName,
          dbus::ObjectPath(kUnityScreenPath));
      scoped_ptr<dbus::MethodCall> method_call;
      method_call.reset(
          new dbus::MethodCall(kUnityScreenInterface, "removeDisplayOnRequest"));
      dbus::MessageWriter message_writer(method_call.get());
      message_writer.AppendInt32(cookie_);
      object_proxy->CallMethodAndBlock(
          method_call.get(), dbus::ObjectProxy::TIMEOUT_USE_DEFAULT);
      cookie_ = 0;
    }

    if (bus_.get()) {
      bus_->ShutdownAndBlock();
      bus_ = NULL;
    }
  } else {
    NOTIMPLEMENTED();
  }
}

void PowerSaveBlocker::ApplicationStateChanged() {
  BrowserPlatformIntegration::ApplicationState state =
      BrowserPlatformIntegration::GetInstance()->GetApplicationState();
  if ((state == BrowserPlatformIntegration::APPLICATION_STATE_INACTIVE) &&
      (cookie_ != 0)) {
    CleanUp();
  } else if ((state == BrowserPlatformIntegration::APPLICATION_STATE_ACTIVE) &&
      (cookie_ == 0)) {
    Init();
  }
}

PowerSaveBlocker::PowerSaveBlocker()
    : form_factor_(oxide::GetFormFactorHint())
    , cookie_(0) {}

content::PowerSaveBlockerOxideDelegate* CreatePowerSaveBlocker(
    content::PowerSaveBlocker::PowerSaveBlockerType type,
    const std::string& reason) {
  return new PowerSaveBlocker();
}

} // namespace oxide
