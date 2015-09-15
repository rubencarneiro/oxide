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

#ifndef _OXIDE_QT_CORE_BROWSER_FIND_CONTROLLER_H_
#define _OXIDE_QT_CORE_BROWSER_FIND_CONTROLLER_H_

#include <QPointer>

#include "base/macros.h"
#include "shared/browser/oxide_find_controller.h"
#include "shared/browser/oxide_find_controller_client.h"

class OxideQFindController;

namespace content {
class WebContents;
}

namespace oxide {
namespace qt {

class FindController : public oxide::FindControllerClient {
 public:
  FindController(OxideQFindController* api_handle);
  ~FindController() override;

  // Initialize FindController from the specified WebContents
  void Init(content::WebContents* contents);

  // Returns true when FindController has been initialized for a WebContents
  bool IsInitialized() const;

  // The following calls proxy through to oxide::FindController. They can't
  // be called until we have been initialized
  void StartFinding(const std::string& text, bool case_sensitive);
  void StopFinding();
  void GotoNextMatch();
  void GotoPreviousMatch();

  // The following calls proxy through to oxide::FindController when we have
  // been initialized
  bool IsRequestActive() const;
  const oxide::FindController::Result& GetResult() const;

 private:
  // oxide::FindControllerClient implementation
  void ResultUpdated(oxide::FindControllerClient::UpdateTypes flags) override;

  QPointer<OxideQFindController> api_handle_;

  oxide::FindController* find_controller_;

  DISALLOW_COPY_AND_ASSIGN(FindController);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_FIND_CONTROLLER_H_
