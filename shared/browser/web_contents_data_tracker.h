// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _OXIDE_SHARED_BROWSER_UNOWNED_WEB_CONTENTS_USER_DATA_H_
#define _OXIDE_SHARED_BROWSER_UNOWNED_WEB_CONTENTS_USER_DATA_H_

#include <memory>

#include "base/logging.h"
#include "base/supports_user_data.h"
#include "content/public/browser/web_contents.h"

namespace oxide {

template <class T>
class WebContentsDataTracker {
 public:
  static std::unique_ptr<T> CreateForWebContents(
      content::WebContents* contents) {
    DCHECK(!FromWebContents(contents));
    DCHECK(contents);

    std::unique_ptr<T> r(new T(contents));
    contents->SetUserData(UserDataKey(), new Data(r.get()));
    r->WebContentsDataTracker<T>::contents_ = contents;
    return std::move(r);
  }

  static T* FromWebContents(content::WebContents* contents) {
    DCHECK(contents);
    Data* data = static_cast<Data*>(contents->GetUserData(UserDataKey()));
    if (!data) {
      return nullptr;
    }
    return data->get();
  }

  virtual ~WebContentsDataTracker() {
    if (contents_) {
      contents_->RemoveUserData(UserDataKey());
    }
    DCHECK(!contents_);
  }

 protected:
  WebContentsDataTracker() = default;

  WebContentsDataTracker(content::WebContents* contents)
      : contents_(contents) {
    contents_->SetUserData(UserDataKey(), new Data(this));
  }

  static inline void* UserDataKey() {
    return &kLocatorKey;
  }

 private:
  class Data : public base::SupportsUserData::Data {
   public:
    Data(T* p) : ptr_(p) {}

    T* get() const { return ptr_; }

   private:
    ~Data() override {
      ptr_->WebContentsDataTracker<T>::contents_ = nullptr;
    }

    T* ptr_;
  };

  content::WebContents* contents_ = nullptr;
  static int kLocatorKey;
};

#define DEFINE_WEB_CONTENTS_DATA_TRACKER_KEY(TYPE) \
template<>                                      \
int WebContentsDataTracker<TYPE>::kLocatorKey = 0

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_UNOWNED_WEB_CONTENTS_USER_DATA_H_
