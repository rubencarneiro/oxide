// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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

#include "oxide_find_controller.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "third_party/WebKit/public/web/WebFindOptions.h"

#include "shared/common/oxide_enum_flags.h"

#include "oxide_find_controller_client.h"

namespace oxide {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(FindController);
OXIDE_MAKE_ENUM_BITWISE_OPERATORS(FindControllerClient::UpdateTypes)

int FindController::s_request_id_counter_ = 0;

FindController::Result::Result()
    : number_of_matches(0),
      active_match_ordinal(0) {}

FindController::FindController(content::WebContents* contents)
    : client_(nullptr),
      contents_(contents),
      case_sensitive_(false),
      current_session_id_(s_request_id_counter_++),
      request_active_(false) {}

FindController::~FindController() {}

// static
FindController* FindController::FromWebContents(
    content::WebContents* contents) {
  return content::WebContentsUserData<FindController>::FromWebContents(contents);
}

void FindController::StartFinding(const std::string& text,
                                  bool case_sensitive) {
  StopFinding();

  if (text.empty()) {
    return;
  }

  text_ = text;
  case_sensitive_ = case_sensitive;
  current_session_id_ = s_request_id_counter_++;
  request_active_ = true;

  blink::WebFindOptions options;
  options.forward = true;
  options.findNext = false;
  options.matchCase = case_sensitive_;

  contents_->Find(current_session_id_, base::UTF8ToUTF16(text_), options);
}

void FindController::StopFinding() {
  request_active_ = false;

  contents_->StopFinding(content::STOP_FIND_ACTION_CLEAR_SELECTION);

  result_ = Result();
  if (client_) {
    client_->ResultUpdated(
        FindControllerClient::UPDATE_TYPE_NUMBER_OF_MATCHES |
          FindControllerClient::UPDATE_TYPE_ACTIVE_MATCH_ORDINAL);
  }
}

void FindController::GotoNextMatch() {
  if (!request_active_) {
    LOG(WARNING) << "Not in an active find-in-page operation";
    return;
  }

  blink::WebFindOptions options;
  options.forward = true;
  options.findNext = true;

  contents_->Find(s_request_id_counter_++, base::UTF8ToUTF16(text_), options);
}

void FindController::GotoPreviousMatch() {
  if (!request_active_) {
    LOG(WARNING) << "Not in an active find-in-page operation";
    return;
  }

  blink::WebFindOptions options;
  options.forward = false;
  options.findNext = true;

  contents_->Find(s_request_id_counter_++, base::UTF8ToUTF16(text_), options);
}

void FindController::HandleFindReply(int request_id,
                                     int number_of_matches,
                                     int active_match_ordinal) {
  if (!request_active_ || request_id < current_session_id_) {
    return;
  }

  Result old_result = result_;

  if (number_of_matches == -1) {
    number_of_matches = old_result.number_of_matches;
  }
  if (active_match_ordinal == -1) {
    active_match_ordinal = old_result.active_match_ordinal;
  }

  result_.number_of_matches = number_of_matches;
  result_.active_match_ordinal = active_match_ordinal;

  if (!client_) {
    return;
  }

  FindControllerClient::UpdateTypes flags =
      FindControllerClient::UPDATE_TYPE_NONE;
  if (result_.number_of_matches != old_result.number_of_matches) {
    flags |= FindControllerClient::UPDATE_TYPE_NUMBER_OF_MATCHES;
  }
  if (result_.active_match_ordinal != old_result.active_match_ordinal) {
    flags |= FindControllerClient::UPDATE_TYPE_ACTIVE_MATCH_ORDINAL;
  }

  if (flags == FindControllerClient::UPDATE_TYPE_NONE) {
    return;
  }

  client_->ResultUpdated(flags);
}

} // namespace oxide
