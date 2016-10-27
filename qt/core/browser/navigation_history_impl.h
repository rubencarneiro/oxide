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

#ifndef _OXIDE_QT_CORE_BROWSER_NAVIGATION_HISTORY_IMPL_H_
#define _OXIDE_QT_CORE_BROWSER_NAVIGATION_HISTORY_IMPL_H_

#include <map>

#include "base/macros.h"

#include "qt/core/glue/navigation_history.h"
#include "shared/browser/navigation_controller_observer.h"

namespace content {
class WebContents;
}

namespace oxide {
namespace qt {

class NavigationHistoryClient;

class NavigationHistoryImpl : public NavigationHistory,
                              public oxide::NavigationControllerObserver {
 public:
  NavigationHistoryImpl(NavigationHistoryClient* client,
                        QObject* handle);
  ~NavigationHistoryImpl() override;

  void RemoveItem(NavigationHistoryItem* item);

 private:
  // NavigationHistory implementation
  void init(WebContentsID web_contents_id) override;
  int getCurrentItemIndex() const override;
  void goToIndex(int index) override;
  void goToOffset(int offset) override;
  int getItemCount() const override;
  int getItemIndex(NavigationHistoryItem* item) const override;
  QExplicitlySharedDataPointer<NavigationHistoryItem> getItemAtIndex(
      int index) override;
  bool canGoBack() const override;
  bool canGoForward() const override;
  bool canGoToOffset(int offset) const override;
  void goBack() override;
  void goForward() override;

  // oxide::NavigationControllerObserver implementation
  void NavigationHistoryChanged() override;

  NavigationHistoryClient* client_;

  content::WebContents* contents_;

  std::map<int, NavigationHistoryItem*> items_;

  DISALLOW_COPY_AND_ASSIGN(NavigationHistoryImpl);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_NAVIGATION_HISTORY_IMPL_H_
