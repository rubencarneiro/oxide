// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_system_notification_dispatcher.h"

#include <algorithm>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gobject/gobject.h>
#include <libnotify/notify.h>

#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/memory/scoped_vector.h"
#include "base/strings/utf_string_conversions.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkUnPreMultiply.h"

#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_platform_integration.h"

#include "oxide_notification_data.h"
#include "oxide_notification_id.h"
#include "oxide_notification_delegate_proxy.h"

namespace oxide {

// Copy&paste from chrome/browser/ui/libgtk2ui/skia_utils_gtk2.cc
GdkPixbuf* SkBitmapToGdkPixbuf(const SkBitmap& bitmap) {
  if (bitmap.drawsNothing()) {
    return nullptr;
  }

  SkAutoLockPixels lock_pixels(bitmap);

  int width = bitmap.width();
  int height = bitmap.height();

  GdkPixbuf* pixbuf =
      gdk_pixbuf_new(GDK_COLORSPACE_RGB,  // The only colorspace gtk supports.
                     true,                // There is an alpha channel.
                     8,
                     width,
                     height);

  // SkBitmaps are premultiplied, we need to unpremultiply them.
  const int kBytesPerPixel = 4;
  uint8_t* divided = gdk_pixbuf_get_pixels(pixbuf);

  for (int y = 0, i = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      uint32_t pixel = *bitmap.getAddr32(x, y);

      int alpha = SkColorGetA(pixel);
      if (alpha != 0 && alpha != 255) {
        SkColor unmultiplied = SkUnPreMultiply::PMColorToColor(pixel);
        divided[i + 0] = SkColorGetR(unmultiplied);
        divided[i + 1] = SkColorGetG(unmultiplied);
        divided[i + 2] = SkColorGetB(unmultiplied);
        divided[i + 3] = alpha;
      } else {
        divided[i + 0] = SkColorGetR(pixel);
        divided[i + 1] = SkColorGetG(pixel);
        divided[i + 2] = SkColorGetB(pixel);
        divided[i + 3] = alpha;
      }
      i += kBytesPerPixel;
    }
  }

  return pixbuf;
}

class SystemNotificationDispatcherLinux;

class SystemNotification {
 public:
  SystemNotification(SystemNotificationDispatcherLinux* dispatcher,
                     const NotificationID& id)
      : dispatcher_(dispatcher),
        id_(id),
        notification_(nullptr) {}
  ~SystemNotification();

  const NotificationID& id() const { return id_; }

  void Update(const base::string16& title,
              const base::string16& body,
              const SkBitmap& icon,
              scoped_refptr<NotificationDelegateProxy> delegate);
  void Close();

 private:
  static void OnClosed(NotifyNotification* notification,
                       gpointer user_data);
  void Closed();

  SystemNotificationDispatcherLinux* dispatcher_;

  NotificationID id_;

  NotifyNotification* notification_;

  scoped_refptr<NotificationDelegateProxy> delegate_;

  DISALLOW_COPY_AND_ASSIGN(SystemNotification);
};

class SystemNotificationDispatcherLinux : public SystemNotificationDispatcher {
 public:
  SystemNotificationDispatcherLinux();
  ~SystemNotificationDispatcherLinux() override;

  void NotificationClosed(SystemNotification* notification);

 private:

  SystemNotification* FindByID(const NotificationID& id);

  // NotificationHandler implementation
  bool DisplayNotification(
      const NotificationID& notification_id,
      const NotificationData& notification_data,
      scoped_refptr<NotificationDelegateProxy> delegate) override;
  void CloseNotification(const NotificationID& notification_id) override;

  ScopedVector<SystemNotification> active_notifications_;

  DISALLOW_COPY_AND_ASSIGN(SystemNotificationDispatcherLinux);
};

// static
void SystemNotification::OnClosed(NotifyNotification* notification,
                                  gpointer user_data) {
  SystemNotification* self = static_cast<SystemNotification*>(user_data);
  DCHECK_EQ(self->notification_, notification);
  self->Closed();
}

void SystemNotification::Closed() {
  if (delegate_) {
    delegate_->NotificationClosed();
  }

  dispatcher_->NotificationClosed(this);
  // |this| has been deleted now
}

SystemNotification::~SystemNotification() {
  if (notification_) {
    g_signal_handlers_disconnect_by_func(notification_,
                                         reinterpret_cast<gpointer>(OnClosed),
                                         this);
    g_object_unref(G_OBJECT(notification_));
    notification_ = nullptr;
  }
}

void SystemNotification::Update(
    const base::string16& title,
    const base::string16& body,
    const SkBitmap& icon,
    scoped_refptr<NotificationDelegateProxy> delegate) {
  std::string title_8 = base::UTF16ToUTF8(title);
  std::string body_8 = base::UTF16ToUTF8(body);

  if (!notification_) {
    notification_ = notify_notification_new(title_8.c_str(),
                                            body_8.c_str(),
                                            nullptr);
    g_signal_connect(notification_, "closed", G_CALLBACK(OnClosed), this);
  } else {
    notify_notification_update(notification_,
                               title_8.c_str(),
                               body_8.c_str(),
                               nullptr);
  }

  GdkPixbuf* pixbuf = SkBitmapToGdkPixbuf(icon);
  notify_notification_set_image_from_pixbuf(notification_, pixbuf);
  if (pixbuf) {
    g_object_unref(G_OBJECT(pixbuf));
  }

  if (delegate_) {
    delegate_->NotificationClosed();
  }

  delegate_ = delegate;

  GError* error = nullptr;
  if (!notify_notification_show(notification_, &error)) {
    LOG(WARNING) << "Failed to show notification: " << error->message;
    g_error_free(error);
    // This prevents us from leaking if we fail to show
    dispatcher_->NotificationClosed(this);
    // |this| has been deleted
    return;
  }

  if (delegate_) {
    delegate_->NotificationDisplayed();
  }
}

void SystemNotification::Close() {
  if (!notify_notification_close(notification_, nullptr)) {
    Closed();
  }
}

SystemNotification* SystemNotificationDispatcherLinux::FindByID(
    const NotificationID& id) {
  auto it = std::find_if(
      active_notifications_.begin(),
      active_notifications_.end(),
      [&id](SystemNotification* n) { return n->id() == id; });
  if (it == active_notifications_.end()) {
    return nullptr;
  }

  return *it;
}

bool SystemNotificationDispatcherLinux::DisplayNotification(
    const NotificationID& id,
    const NotificationData& data,
    scoped_refptr<NotificationDelegateProxy> delegate) {
  SystemNotification* notification = FindByID(id);

  if (!notification) {
    notification = new SystemNotification(this, id);
    active_notifications_.push_back(notification);
  }

  notification->Update(data.title, data.body, data.icon, delegate);
  // |notification| could have been deleted by this point if the call to
  // libnotify failed

  return true;
}

void SystemNotificationDispatcherLinux::CloseNotification(
    const NotificationID& id) {
  SystemNotification* notification = FindByID(id);
  if (!notification) {
    return;
  }

  notification->Close();
}

SystemNotificationDispatcherLinux::SystemNotificationDispatcherLinux() {
  notify_init(
      BrowserPlatformIntegration::GetInstance()->GetApplicationName().c_str());
}

SystemNotificationDispatcherLinux::~SystemNotificationDispatcherLinux() {}

void SystemNotificationDispatcherLinux::NotificationClosed(
    SystemNotification* notification) {
  auto it = std::find(active_notifications_.begin(),
                      active_notifications_.end(),
                      notification);
  DCHECK(it != active_notifications_.end());
  active_notifications_.erase(it);
}

// static
std::unique_ptr<SystemNotificationDispatcher>
SystemNotificationDispatcher::Create() {
  return base::WrapUnique(new SystemNotificationDispatcherLinux());
}

} // namespace oxide
