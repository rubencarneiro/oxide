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

#include "oxide_platform_notification_service.h"

#include <map>
#include <libnotify/notify.h>
#include "third_party/skia/include/core/SkImageEncoder.h"
#include "third_party/skia/include/core/SkBitmap.h"

#include "base/strings/utf_string_conversions.h"
#include "content/public/common/platform_notification_data.h"
#include "content/public/browser/desktop_notification_delegate.h"
#include "third_party/skia/include/core/SkUnPreMultiply.h"

#include "shared/browser/permissions/oxide_temporary_saved_permission_context.h"
#include "shared/browser/oxide_browser_context.h"

namespace oxide {

// Copy&paste from chrome/browser/ui/libgtk2ui/skia_utils_gtk2.cc
// to avoid dependency on full Gdk.
GdkPixbuf* GdkPixbufFromSkBitmap(const SkBitmap& bitmap) {
  if (bitmap.isNull()) {
    return nullptr;
  }

  SkAutoLockPixels lock_pixels(bitmap);

  int width = bitmap.width();
  int height = bitmap.height();

  GdkPixbuf* pixbuf =
      gdk_pixbuf_new(GDK_COLORSPACE_RGB,  // The only colorspace gtk supports.
                     TRUE,                // There is an alpha channel.
                     8,
                     width,
                     height);

  // SkBitmaps are premultiplied, we need to unpremultiply them.
  const int kBytesPerPixel = 4;
  uint8_t* divided = gdk_pixbuf_get_pixels(pixbuf);

  for (int y = 0, i = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      uint32_t pixel = bitmap.getAddr32(0, y)[x];

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


namespace {

blink::WebNotificationPermission
ToNotificationPermission(TemporarySavedPermissionStatus status) {
  switch (status) {
  case TEMPORARY_SAVED_PERMISSION_STATUS_ALLOWED:
    return blink::WebNotificationPermissionAllowed;
  case TEMPORARY_SAVED_PERMISSION_STATUS_DENIED:
    return blink::WebNotificationPermissionDenied;
  default:
    return blink::WebNotificationPermissionDefault;
  };
}

class DesktopNotification: public base::RefCounted<DesktopNotification> {
public:
  base::Closure* GetCancelCallback();

  static scoped_refptr<DesktopNotification> CreateDesktopNotification(
      const GURL& origin, const std::string &tag);
  void Initialize(
      const base::string16& title, const base::string16& body,
      const SkBitmap& icon,
      scoped_ptr<content::DesktopNotificationDelegate> delegate);
private:
  DesktopNotification(const GURL &origin, const std::string &tag);
  ~DesktopNotification();

  void CancelNotification();

  GURL origin_;
  std::string tag_;
  base::Closure cancel_callback_;
  NotifyNotification *notification_;
  scoped_ptr<content::DesktopNotificationDelegate> delegate_;

  static void OnClosed(NotifyNotification *notification, DesktopNotification *self);
  static std::map<GURL, std::map<std::string, scoped_refptr<DesktopNotification>>> table_;

  friend class base::RefCounted<DesktopNotification>;
};

std::map<GURL, std::map<std::string, scoped_refptr<DesktopNotification>>> DesktopNotification::table_;

DesktopNotification::DesktopNotification(const GURL &origin, const std::string &tag)
  : origin_(origin), tag_(tag), notification_(nullptr) {
}

scoped_refptr<DesktopNotification>
DesktopNotification::CreateDesktopNotification(const GURL& origin, const std::string &tag) {
  table_[origin][tag] = new DesktopNotification(origin, tag);
  return table_[origin][tag];
}

void
DesktopNotification::Initialize(
    const base::string16& title, const base::string16& body,
    const SkBitmap& icon,
    scoped_ptr<content::DesktopNotificationDelegate> delegate) {
  DCHECK(!notification_);

  delegate_ = delegate.Pass();

  std::string t = base::UTF16ToUTF8(title);
  std::string b = base::UTF16ToUTF8(body);

  notification_ = notify_notification_new(t.c_str(), b.c_str(), nullptr);

  g_signal_connect(notification_, "closed", G_CALLBACK(OnClosed), this);

  if (delegate_.get()) {
    delegate_->NotificationDisplayed();
  }

  if (icon.width() && icon.height()) {
    GdkPixbuf *pix = GdkPixbufFromSkBitmap(icon);
    notify_notification_set_image_from_pixbuf(notification_, pix);
    g_object_unref(G_OBJECT(pix));
  }
  notify_notification_show(notification_, nullptr);
}

base::Closure*
DesktopNotification::GetCancelCallback() {
  cancel_callback_ = base::Bind(&DesktopNotification::CancelNotification, this);
  return &cancel_callback_;
}

DesktopNotification::~DesktopNotification() {
  g_object_unref(G_OBJECT(notification_));
}

void
DesktopNotification::CancelNotification() {
  table_[origin_].erase(tag_);
  this->Release();
}

void
DesktopNotification::OnClosed(NotifyNotification *notification, DesktopNotification *self) {
  table_[self->origin_].erase(self->tag_);
  if (self->delegate_.get()) {
    self->delegate_->NotificationClosed();
  }
  self->Release();
}

}

void
PlatformNotificationService::Initialize(const std::string& name) {
  notify_init(name.c_str());
}

blink::WebNotificationPermission
PlatformNotificationService::CheckPermissionOnIOThread(
    content::ResourceContext* resource_context,
    const GURL& origin,
    int render_process_id) {
  TemporarySavedPermissionContext* permission_context =
    BrowserContextIOData::FromResourceContext(resource_context)->GetTemporarySavedPermissionContext();

  return ToNotificationPermission(permission_context->GetPermissionStatus(
           TEMPORARY_SAVED_PERMISSION_TYPE_NOTIFICATIONS, origin, origin));
}

blink::WebNotificationPermission
PlatformNotificationService::CheckPermissionOnUIThread(
    content::BrowserContext* browser_context,
    const GURL& origin,
    int render_process_id) {
  BrowserContext* context = static_cast<BrowserContext*>(browser_context);
  TemporarySavedPermissionContext* permission_context = context->GetTemporarySavedPermissionContext();

  return ToNotificationPermission(permission_context->GetPermissionStatus(
           TEMPORARY_SAVED_PERMISSION_TYPE_NOTIFICATIONS, origin, origin));
}

void
PlatformNotificationService::DisplayNotification(
    content::BrowserContext* browser_context,
    const GURL& origin,
    const SkBitmap& icon,
    const content::PlatformNotificationData& notification_data,
    scoped_ptr<content::DesktopNotificationDelegate> delegate,
    base::Closure* cancel_callback) {

  scoped_refptr<DesktopNotification> notification = DesktopNotification::CreateDesktopNotification(
    origin, notification_data.tag);
  notification->Initialize(notification_data.title, notification_data.body, icon, delegate.Pass());

  cancel_callback = notification->GetCancelCallback();
}

void
PlatformNotificationService::DisplayPersistentNotification(
    content::BrowserContext* browser_context,
    int64 service_worker_registration_id,
    const GURL& origin,
    const SkBitmap& icon,
    const content::PlatformNotificationData& notification_data) {
  NOTIMPLEMENTED();
}

void
PlatformNotificationService::ClosePersistentNotification(content::BrowserContext* browser_context,
    int64_t persistent_notification_id) {
  NOTIMPLEMENTED();
}

bool
PlatformNotificationService::GetDisplayedPersistentNotifications(
    content::BrowserContext* browser_context,
    std::set<std::string>* displayed_notifications) {
  return false;
}

PlatformNotificationService*
PlatformNotificationService::GetInstance() {
  return Singleton<PlatformNotificationService>::get();
}

}
