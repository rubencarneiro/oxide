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

#include "navigation_controller_observer.h"

#include "base/observer_list.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

namespace oxide {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(NavigationControllerObserver::Delegate);

class NavigationControllerObserver::Delegate
    : public content::WebContentsUserData<Delegate>,
      public content::WebContentsObserver,
      public content::NotificationObserver {
 public:
  Delegate(content::WebContents* contents);
  ~Delegate() override;

  void AddObserver(NavigationControllerObserver* observer);
  void RemoveObserver(NavigationControllerObserver* observer);

  void NotifyNavigationStateChanged(content::InvalidateTypes change_flags);

 private:
  void DispatchNavigationHistoryChanged();

  // content::WebContentsObserver implementation
  void DidStartNavigationToPendingEntry(
      const GURL& url,
      content::ReloadType reload_type) override;

  // content::NotificationObserver implementation
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  content::NotificationRegistrar notification_registrar_;

  base::ObserverList<NavigationControllerObserver> observer_list_;
};

void NavigationControllerObserver::Delegate::DispatchNavigationHistoryChanged() {
  FOR_EACH_OBSERVER(NavigationControllerObserver,
                    observer_list_,
                    NavigationHistoryChanged());
}

void NavigationControllerObserver::Delegate::DidStartNavigationToPendingEntry(
    const GURL& url,
    content::ReloadType reload_type) {
  DispatchNavigationHistoryChanged();
}

void NavigationControllerObserver::Delegate::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  if (content::Source<content::NavigationController>(source).ptr() !=
      &(web_contents()->GetController())) {
    return;
  }

  switch (type) {
    case content::NOTIFICATION_NAV_LIST_PRUNED:
    case content::NOTIFICATION_NAV_ENTRY_PENDING:
      DispatchNavigationHistoryChanged();
      return;
    default:
      break;
  }
}

NavigationControllerObserver::Delegate::Delegate(content::WebContents* contents)
    : content::WebContentsObserver(contents) {
  notification_registrar_.Add(
      this, content::NOTIFICATION_NAV_LIST_PRUNED,
      content::NotificationService::AllSources());
  // SetPendingEntry can clear an existing transient or pending entry, which can
  // change the current entry index
  notification_registrar_.Add(
      this, content::NOTIFICATION_NAV_ENTRY_PENDING,
      content::NotificationService::AllSources());
}

NavigationControllerObserver::Delegate::~Delegate() {
  FOR_EACH_OBSERVER(NavigationControllerObserver,
                    observer_list_,
                    OnDelegateDestruction());
}

void NavigationControllerObserver::Delegate::AddObserver(
    NavigationControllerObserver* observer) {
  observer_list_.AddObserver(observer);
}

void NavigationControllerObserver::Delegate::RemoveObserver(
    NavigationControllerObserver* observer) {
  observer_list_.RemoveObserver(observer);
}

void NavigationControllerObserver::Delegate::NotifyNavigationStateChanged(
    content::InvalidateTypes change_flags) {
  if (!(change_flags & (content::INVALIDATE_TYPE_URL |
                        content::INVALIDATE_TYPE_TAB |
                        content::INVALIDATE_TYPE_LOAD))) {
    return;
  }

  DispatchNavigationHistoryChanged();
}

void NavigationControllerObserver::Observe(
    content::NavigationController* controller) {
  if (delegate_) {
    delegate_->RemoveObserver(this);
    delegate_ = nullptr;
  }

  if (!controller) {
    return;
  }

  content::WebContents* contents = controller->GetWebContents();
  Delegate::CreateForWebContents(contents);
  delegate_ = Delegate::FromWebContents(contents);
  delegate_->AddObserver(this);
}

void NavigationControllerObserver::OnDelegateDestruction() {
  delegate_ = nullptr;
}

NavigationControllerObserver::NavigationControllerObserver() = default;

NavigationControllerObserver::NavigationControllerObserver(
    content::NavigationController* controller) {
  Observe(controller);
}

NavigationControllerObserver::~NavigationControllerObserver() {
  if (delegate_) {
    delegate_->RemoveObserver(this);
  }
}

// static
void NavigationControllerObserver::NotifyNavigationStateChanged(
    const content::NavigationController& controller,
    content::InvalidateTypes change_flags) {
  Delegate* delegate = Delegate::FromWebContents(controller.GetWebContents());
  if (!delegate) {
    return;
  }

  delegate->NotifyNavigationStateChanged(change_flags);
}

} // namespace oxide
