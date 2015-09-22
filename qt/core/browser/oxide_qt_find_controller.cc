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

#include "oxide_qt_find_controller.h"

#include "base/logging.h"

#include "qt/core/api/oxideqfindcontroller.h"
#include "shared/browser/oxide_find_controller.h"

namespace oxide {
namespace qt {

void FindController::ResultUpdated(
    oxide::FindControllerClient::UpdateTypes flags) {
  if (flags & oxide::FindControllerClient::UPDATE_TYPE_NUMBER_OF_MATCHES) {
    Q_EMIT api_handle_->countChanged();
  }
  if (flags & oxide::FindControllerClient::UPDATE_TYPE_ACTIVE_MATCH_ORDINAL) {
    Q_EMIT api_handle_->currentChanged();
  }
}

FindController::FindController(OxideQFindController* api_handle)
    : api_handle_(api_handle),
      find_controller_(nullptr) {}

FindController::~FindController() {
  if (find_controller_) {
    find_controller_->set_client(nullptr);
  }
}

void FindController::Init(content::WebContents* contents) {
  DCHECK(!find_controller_);
  find_controller_ = oxide::FindController::FromWebContents(contents);
  find_controller_->set_client(this);
}

bool FindController::IsInitialized() const {
  return !!find_controller_;
}

void FindController::StartFinding(const std::string& text,
                                  bool case_sensitive) {
  DCHECK(find_controller_);
  find_controller_->StartFinding(text, case_sensitive);
}

void FindController::StopFinding() {
  DCHECK(find_controller_);
  find_controller_->StopFinding();
}

void FindController::GotoNextMatch() {
  DCHECK(find_controller_);
  find_controller_->GotoNextMatch();
}

void FindController::GotoPreviousMatch() {
  DCHECK(find_controller_);
  find_controller_->GotoPreviousMatch();
}

bool FindController::IsRequestActive() const {
  if (!find_controller_) {
    return false;
  }

  return find_controller_->request_active();
}

const oxide::FindController::Result& FindController::GetResult() const {
  if (!find_controller_) {
    static oxide::FindController::Result g_empty;
    return g_empty;
  }
    
  return find_controller_->result();
}

} // namespace qt
} // namespace oxide
