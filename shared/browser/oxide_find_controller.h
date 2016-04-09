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

#ifndef _OXIDE_SHARED_BROWSER_FIND_CONTROLLER_H_
#define _OXIDE_SHARED_BROWSER_FIND_CONTROLLER_H_

#include <string>

#include "base/macros.h"
#include "content/public/browser/web_contents_user_data.h"

#include "shared/common/oxide_shared_export.h"

namespace content {
class WebContents;
}

namespace oxide {

class FindControllerClient;

class OXIDE_SHARED_EXPORT FindController
    : public content::WebContentsUserData<FindController> {
 public:
  ~FindController() override;

  static FindController* FromWebContents(content::WebContents* contents);

  // Begin a find-in-page for the current |text|. |case_sensitive| specifies
  // whether the find will be case sensitive
  void StartFinding(const std::string& text, bool case_sensitive);

  // Stop the current find-in-page
  void StopFinding();

  // Go to the next match
  void GotoNextMatch();

  // Go to the previous match
  void GotoPreviousMatch();

  // Returns whether there is an active find-in-page request
  bool request_active() const { return request_active_; }

  // Return the text being searched for
  std::string text() const { return text_; }

  // Whether the search is case sensitive
  bool case_sensitive() const { return case_sensitive_; }

  void set_client(FindControllerClient* client) {
    client_ = client;
  }

  struct Result {
    Result();

    int number_of_matches;
    int active_match_ordinal;
  };

  const Result& result() const { return result_; }

  void HandleFindReply(int request_id,
                       int number_of_matches,
                       int active_match_ordinal);

 private:
  friend class content::WebContentsUserData<FindController>;

  FindController(content::WebContents* contents);

  FindControllerClient* client_;

  content::WebContents* contents_;

  std::string text_;
  bool case_sensitive_;

  static int s_request_id_counter_;

  int current_request_id_;
  bool request_active_;

  Result result_;

  DISALLOW_COPY_AND_ASSIGN(FindController);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_FIND_CONTROLLER_H_
