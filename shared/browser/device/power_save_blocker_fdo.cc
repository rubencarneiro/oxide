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

#include "power_save_blocker_fdo.h"

#include <memory>

#include "base/bind.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_path.h"
#include "dbus/object_proxy.h"

#include "shared/browser/oxide_browser_platform_integration.h"

namespace oxide {

namespace {
const uint32_t kInvalidCookie = uint32_t(-1);

const char kFreeDesktopScreenSaverName[] = "org.freedesktop.ScreenSaver";
const char kFreeDesktopScreenSaverPath[] = "/org/freedesktop/ScreenSaver";
const char kFreeDesktopScreenSaverInterface[] = "org.freedesktop.ScreenSaver";
}

class PowerSaveBlockerFDO::Core : public base::RefCountedThreadSafe<Core> {
 public:
  Core(device::PowerSaveBlocker::PowerSaveBlockerType type,
       const std::string& description)
      : type_(type),
        description_(description) {}

  void Init();
  void CleanUp();

 private:
  friend class base::RefCountedThreadSafe<Core>;
  ~Core();

  void ApplyBlock();
  void RemoveBlock();

  device::PowerSaveBlocker::PowerSaveBlockerType type_;
  std::string description_;

  std::string application_name_;

  scoped_refptr<dbus::Bus> bus_;

  uint32_t cookie_;

  DISALLOW_COPY_AND_ASSIGN(Core);
};

PowerSaveBlockerFDO::Core::~Core() {
  DCHECK_EQ(cookie_, kInvalidCookie);
  DCHECK(!bus_);
}

void PowerSaveBlockerFDO::Core::ApplyBlock() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::FILE);

  DCHECK(!bus_.get());
  dbus::Bus::Options options;
  options.bus_type = dbus::Bus::SESSION;
  options.connection_type = dbus::Bus::PRIVATE;
  bus_ = new dbus::Bus(options);

  scoped_refptr<dbus::ObjectProxy> object_proxy =
      bus_->GetObjectProxy(
          kFreeDesktopScreenSaverName,
          dbus::ObjectPath(kFreeDesktopScreenSaverPath));
  std::unique_ptr<dbus::MethodCall> method_call =
      base::MakeUnique<dbus::MethodCall>(kFreeDesktopScreenSaverInterface,
                                         "Inhibit");

  dbus::MessageWriter message_writer(method_call.get());
  message_writer.AppendString(application_name_);
  message_writer.AppendString(description_);

  std::unique_ptr<dbus::Response> response =
      object_proxy->CallMethodAndBlock(method_call.get(),
                                       dbus::ObjectProxy::TIMEOUT_USE_DEFAULT);
  if (response) {
    dbus::MessageReader message_reader(response.get());
    if (!message_reader.PopUint32(&cookie_)) {
      LOG(ERROR) << "Invalid response for screen blanking inhibition request: "
                 << response->ToString();
    }
  } else {
    LOG(ERROR) << "Failed to inhibit screen blanking";
  }
}

void PowerSaveBlockerFDO::Core::RemoveBlock() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::FILE);

  if (cookie_ != kInvalidCookie) {
    DCHECK(bus_.get());
    scoped_refptr<dbus::ObjectProxy> object_proxy =
        bus_->GetObjectProxy(kFreeDesktopScreenSaverName,
                             dbus::ObjectPath(kFreeDesktopScreenSaverPath));
    std::unique_ptr<dbus::MethodCall> method_call =
        base::MakeUnique<dbus::MethodCall>(kFreeDesktopScreenSaverInterface,
                                           "UnInhibit");
    dbus::MessageWriter message_writer(method_call.get());
    message_writer.AppendUint32(cookie_);

    object_proxy->CallMethodAndBlock(method_call.get(),
                                     dbus::ObjectProxy::TIMEOUT_USE_DEFAULT);
    cookie_ = kInvalidCookie;
  }

  if (bus_.get()) {
    bus_->ShutdownAndBlock();
    bus_ = nullptr;
  }
}

void PowerSaveBlockerFDO::Core::Init() {
  application_name_ =
      BrowserPlatformIntegration::GetInstance()->GetApplicationName();

  content::BrowserThread::PostTask(content::BrowserThread::FILE,
                                   FROM_HERE,
                                   base::Bind(&Core::ApplyBlock, this));
}

void PowerSaveBlockerFDO::Core::CleanUp() {
  content::BrowserThread::PostTask(content::BrowserThread::FILE,
                                   FROM_HERE,
                                   base::Bind(&Core::RemoveBlock, this));
}

PowerSaveBlockerFDO::PowerSaveBlockerFDO(
    device::PowerSaveBlocker::PowerSaveBlockerType type,
    const std::string& description)
    : core_(new Core(type, description)) {
  core_->Init();
}

PowerSaveBlockerFDO::~PowerSaveBlockerFDO() {
  core_->CleanUp();
}

} // namespace oxide
