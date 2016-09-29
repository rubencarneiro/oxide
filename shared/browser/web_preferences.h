// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_WEB_PREFERENCES_H_
#define _OXIDE_SHARED_BROWSER_WEB_PREFERENCES_H_

#include <string>

#include "base/callback.h"
#include "base/callback_list.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/public/common/web_preferences.h"

#include "shared/common/oxide_shared_export.h"

namespace oxide {

class OXIDE_SHARED_EXPORT WebPreferences
    : public base::RefCounted<WebPreferences>{
 public:
  WebPreferences();

#define WEB_PREF(type, name, initial_value) \
  const type& name() const { return name##_; } \
  void set_##name(const type&);
#include "shared/browser/web_preferences_list.h"
#undef WEB_PREF

  void ApplyToWebkitPrefs(content::WebPreferences* prefs) const;

  scoped_refptr<WebPreferences> Clone();

  using ObserverCallback = base::Callback<void()>;
  using Subscription = base::CallbackList<void()>::Subscription;

  std::unique_ptr<Subscription> AddChangeCallback(
      const ObserverCallback& callback);

 private:
  friend class base::RefCounted<WebPreferences>;
  ~WebPreferences();

#define WEB_PREF(type, name, initial_value) \
  type name##_;
#include "shared/browser/web_preferences_list.h"
#undef WEB_PREF

  bool dummy_;

  base::CallbackList<void()> callback_list_;

  DISALLOW_COPY_AND_ASSIGN(WebPreferences);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_PREFERENCES_H_
