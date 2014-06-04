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

#include "content/browser/power_save_blocker_impl.h"

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_thread.h"
#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_manager.h"
#include "dbus/object_path.h"
#include "dbus/object_proxy.h"

namespace content {

namespace {

const char kPowerdServiceName[] = "com.canonical.powerd";
const char kPowerdPath[] = "/com/canonical/powerd";
const char kPowerdInterface[] = "com.canonical.powerd";

}

class PowerSaveBlockerImpl::Delegate
    : public base::RefCountedThreadSafe<PowerSaveBlockerImpl::Delegate> {
 public:
  Delegate(PowerSaveBlockerType type, const std::string& reason);

  void Init();
  void CleanUp();

 private:
  friend class base::RefCountedThreadSafe<Delegate>;
  ~Delegate() {}

  void ApplyBlock();
  void RemoveBlock();

  PowerSaveBlockerType type_;
  std::string reason_;
  scoped_refptr<dbus::Bus> bus_;
  std::string cookie_;

  DISALLOW_COPY_AND_ASSIGN(Delegate);
};

PowerSaveBlockerImpl::Delegate::Delegate(PowerSaveBlockerType type,
                                         const std::string& reason) :
    type_(type),
    reason_(reason) {}

void PowerSaveBlockerImpl::Delegate::Init() {
  BrowserThread::PostTask(BrowserThread::FILE, FROM_HERE,
                          base::Bind(&Delegate::ApplyBlock, this));
}

void PowerSaveBlockerImpl::Delegate::CleanUp() {
  BrowserThread::PostTask(BrowserThread::FILE, FROM_HERE,
                          base::Bind(&Delegate::RemoveBlock, this));
}

void PowerSaveBlockerImpl::Delegate::ApplyBlock() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));
  DCHECK(!bus_.get());

  dbus::Bus::Options options;
  options.bus_type = dbus::Bus::SYSTEM;
  options.connection_type = dbus::Bus::PRIVATE;
  bus_ = new dbus::Bus(options);

  scoped_refptr<dbus::ObjectManager> object_manager = bus_->GetObjectManager(
      kPowerdServiceName, dbus::ObjectPath(kPowerdPath));
  // FIXME: how can I check if powerd is available without actually making a
  // dbus call to its interface that will result in an ugly error message in
  // the console if not available?
  if (object_manager->GetObjectsWithInterface(kPowerdInterface).empty()) {
    return;
  }

  scoped_refptr<dbus::ObjectProxy> object_proxy = bus_->GetObjectProxy(
        kPowerdServiceName,
        dbus::ObjectPath(kPowerdPath));
  scoped_ptr<dbus::MethodCall> method_call;
  method_call.reset(
      new dbus::MethodCall(kPowerdInterface, "requestDisplayState"));
  scoped_ptr<dbus::MessageWriter> message_writer;
  message_writer.reset(new dbus::MessageWriter(method_call.get()));
  message_writer->AppendString("[oxide] " + reason_);
  message_writer->AppendInt32(1); // POWERD_DISPLAY_STATE_ON
  message_writer->AppendUint32(4); // POWERD_DISPLAY_FLAG_BRIGHT

  scoped_ptr<dbus::Response> response(object_proxy->CallMethodAndBlock(
      method_call.get(), dbus::ObjectProxy::TIMEOUT_USE_DEFAULT));
  if (response) {
    dbus::MessageReader message_reader(response.get());
    if (!message_reader.PopString(&cookie_)) {
      LOG(ERROR) << "Invalid response for screen blanking inhibition request: "
                 << response->ToString();
    }
  } else {
    LOG(ERROR) << "Failed to inhibit screen blanking";
  }
}

void PowerSaveBlockerImpl::Delegate::RemoveBlock() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));
  DCHECK(bus_.get());

  if (!cookie_.empty()) {
    scoped_refptr<dbus::ObjectProxy> object_proxy = bus_->GetObjectProxy(
        kPowerdServiceName,
        dbus::ObjectPath(kPowerdPath));
    scoped_ptr<dbus::MethodCall> method_call;
    method_call.reset(
        new dbus::MethodCall(kPowerdInterface, "clearDisplayState"));
    dbus::MessageWriter message_writer(method_call.get());
    message_writer.AppendString(cookie_);
    object_proxy->CallMethodAndBlock(
        method_call.get(), dbus::ObjectProxy::TIMEOUT_USE_DEFAULT);
    cookie_.clear();
  }
  
  bus_->ShutdownAndBlock();
  bus_ = NULL;
}

PowerSaveBlockerImpl::PowerSaveBlockerImpl(PowerSaveBlockerType type,
                                           const std::string& reason)
    : delegate_(new Delegate(type, reason)) {
  delegate_->Init();
}

PowerSaveBlockerImpl::~PowerSaveBlockerImpl() {
  delegate_->CleanUp();
}

}
