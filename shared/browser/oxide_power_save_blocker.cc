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

#include "shared/common/oxide_form_factor.h"
#include "shared/port/content/browser/power_save_blocker_oxide.h"

#include "oxide_browser_platform_integration.h"
#include "oxide_browser_platform_integration_observer.h"

namespace oxide {

namespace {

const char kUnityScreenServiceName[] = "com.canonical.Unity.Screen";
const char kUnityScreenPath[] = "/com/canonical/Unity/Screen";
const char kUnityScreenInterface[] = "com.canonical.Unity.Screen";
const int kInvalidCookie = -1;

const char kFreeDesktopScreenSaverName[] = "org.freedesktop.ScreenSaver";
const char kFreeDesktopScreenSaverPath[] = "/org/freedesktop/ScreenSaver";
const char kFreeDestopScreenSaverInterface[] = "org.freedesktop.ScreenSaver";

const char kDefaultInhibitReason[] = "Active Application";

}

class PowerSaveBlocker : public content::PowerSaveBlockerOxideDelegate,
                         public BrowserPlatformIntegrationObserver {
 public:
  PowerSaveBlocker(const std::string& description);

 private:
  virtual ~PowerSaveBlocker() {}

  void Init() final;
  void CleanUp() final;

  void ApplyBlock();
  void RemoveBlock();

  void ApplyBlockUnityScreenService();
  void RemoveBlockUnityScreenService();

  void ApplyBlockFreedesktop();
  void RemoveBlockFreedesktop();

  // BrowserPlatformIntegrationObserver implementation
  void ApplicationStateChanged() final;

  oxide::FormFactor form_factor_;
  scoped_refptr<dbus::Bus> bus_;
  union {
    int32_t unity_cookie_;
    uint32_t freedesktop_cookie_;
  };
  std::string description_;
};

void PowerSaveBlocker::Init() {
  if (BrowserPlatformIntegration::GetInstance()->GetApplicationState() !=
      BrowserPlatformIntegration::APPLICATION_STATE_SUSPENDED) {
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
    ApplyBlockUnityScreenService();
  } else {
    ApplyBlockFreedesktop();
  }
}

void PowerSaveBlocker::RemoveBlock() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::FILE));

  if (form_factor_ == oxide::FORM_FACTOR_PHONE ||
      form_factor_ == oxide::FORM_FACTOR_TABLET) {
    RemoveBlockUnityScreenService();
  } else {
    RemoveBlockFreedesktop();
  }
}

void PowerSaveBlocker::ApplyBlockUnityScreenService() {
  if (unity_cookie_ != kInvalidCookie) {
    return;
  }

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
    if (!message_reader.PopInt32(&unity_cookie_)) {
      LOG(ERROR) << "Invalid response for screen blanking inhibition request: "
                 << response->ToString();
    }
  } else {
    LOG(ERROR) << "Failed to inhibit screen blanking";
  }
}

void PowerSaveBlocker::RemoveBlockUnityScreenService() {
  if (unity_cookie_ != kInvalidCookie) {
    DCHECK(bus_.get());
    scoped_refptr<dbus::ObjectProxy> object_proxy = bus_->GetObjectProxy(
        kUnityScreenServiceName,
        dbus::ObjectPath(kUnityScreenPath));
    scoped_ptr<dbus::MethodCall> method_call;
    method_call.reset(
        new dbus::MethodCall(kUnityScreenInterface, "removeDisplayOnRequest"));
    dbus::MessageWriter message_writer(method_call.get());
    message_writer.AppendInt32(unity_cookie_);
    object_proxy->CallMethodAndBlock(
        method_call.get(), dbus::ObjectProxy::TIMEOUT_USE_DEFAULT);
    unity_cookie_ = kInvalidCookie;
  }

  if (bus_.get()) {
    bus_->ShutdownAndBlock();
    bus_ = nullptr;
  }
}

void PowerSaveBlocker::ApplyBlockFreedesktop() {
  if (freedesktop_cookie_ != uint32_t(kInvalidCookie)) {
    return;
  }

  std::string application_name =
      BrowserPlatformIntegration::GetInstance()->GetApplicationName();
  std::string description{kDefaultInhibitReason};

  if (!description_.empty()) {
    description = description_;
  }

  DCHECK(!bus_.get());
  dbus::Bus::Options options;
  options.bus_type = dbus::Bus::SESSION;
  options.connection_type = dbus::Bus::PRIVATE;
  bus_ = new dbus::Bus(options);

  scoped_refptr<dbus::ObjectProxy> object_proxy = bus_->GetObjectProxy(
        kFreeDesktopScreenSaverName,
        dbus::ObjectPath(kFreeDesktopScreenSaverPath));
  scoped_ptr<dbus::MethodCall> method_call;
  method_call.reset(
      new dbus::MethodCall(kFreeDestopScreenSaverInterface, "Inhibit"));
  scoped_ptr<dbus::MessageWriter> message_writer;
  message_writer.reset(new dbus::MessageWriter(method_call.get()));
  message_writer->AppendString(application_name);
  message_writer->AppendString(description);

  scoped_ptr<dbus::Response> response(object_proxy->CallMethodAndBlock(
      method_call.get(), dbus::ObjectProxy::TIMEOUT_USE_DEFAULT));
  if (response) {
    dbus::MessageReader message_reader(response.get());
    if (!message_reader.PopUint32(&freedesktop_cookie_)) {
      LOG(ERROR) << "Invalid response for screen blanking inhibition request: "
                 << response->ToString();
    }
  } else {
    LOG(ERROR) << "Failed to inhibit screen blanking";
  }
}

void PowerSaveBlocker::RemoveBlockFreedesktop() {
  if (freedesktop_cookie_ != uint32_t(kInvalidCookie)) {
    DCHECK(bus_.get());
    scoped_refptr<dbus::ObjectProxy> object_proxy = bus_->GetObjectProxy(
        kFreeDesktopScreenSaverName,
        dbus::ObjectPath(kFreeDesktopScreenSaverPath));
    scoped_ptr<dbus::MethodCall> method_call;
    method_call.reset(
        new dbus::MethodCall(kFreeDestopScreenSaverInterface, "UnInhibit"));
    dbus::MessageWriter message_writer(method_call.get());
    message_writer.AppendUint32(freedesktop_cookie_);
    object_proxy->CallMethodAndBlock(
        method_call.get(), dbus::ObjectProxy::TIMEOUT_USE_DEFAULT);
    freedesktop_cookie_ = kInvalidCookie;
  }

  if (bus_.get()) {
    bus_->ShutdownAndBlock();
    bus_ = nullptr;
  }
}

void PowerSaveBlocker::ApplicationStateChanged() {
  BrowserPlatformIntegration::ApplicationState state =
      BrowserPlatformIntegration::GetInstance()->GetApplicationState();
  if (state == BrowserPlatformIntegration::APPLICATION_STATE_SUSPENDED) {
    CleanUp();
  } else {
    Init();
  }
}

PowerSaveBlocker::PowerSaveBlocker(const std::string& description)
    : form_factor_(oxide::GetFormFactorHint()),
      unity_cookie_(kInvalidCookie),
      description_(description) {
  Attach();
}

content::PowerSaveBlockerOxideDelegate* CreatePowerSaveBlocker(
    content::PowerSaveBlocker::PowerSaveBlockerType type,
    content::PowerSaveBlocker::Reason reason,
    const std::string& description) {
  return new PowerSaveBlocker(description);
}

} // namespace oxide
