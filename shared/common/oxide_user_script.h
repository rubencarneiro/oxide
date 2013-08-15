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

#ifndef _OXIDE_SHARED_COMMON_USER_SCRIPT_H_
#define _OXIDE_SHARED_COMMON_USER_SCRIPT_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "extensions/common/url_pattern_set.h"
#include "url/gurl.h"

class Pickle;
class PickleIterator;
class URLPattern;

namespace oxide {

class UserScript FINAL {
 public:

  UserScript();

  enum RunLocation {
    DOCUMENT_START,
    DOCUMENT_END,
    DOCUMENT_IDLE,
    RUN_LOCATION_LAST
  };

  GURL url() const {
    return url_;
  }
  void set_url(const GURL& url) {
    url_ = url;
  }

  const std::vector<std::string>& exclude_globs() const {
    return exclude_globs_;
  }
  void add_exclude_glob(const std::string& glob);

  const std::vector<std::string>& include_globs() const {
    return include_globs_;
  }
  void add_include_glob(const std::string& glob);

  const extensions::URLPatternSet& include_url_set() const {
    return include_pattern_set_;
  }
  void add_include_url_pattern(const URLPattern& pattern);

  const extensions::URLPatternSet& exclude_url_set() const {
    return exclude_pattern_set_;
  }
  void add_exclude_url_pattern(const URLPattern& pattern);

  RunLocation run_location() const {
    return run_location_;
  }
  void set_run_location(RunLocation run_location) {
    run_location_ = run_location;
  }

  bool match_all_frames() const {
    return match_all_frames_;
  }
  void set_match_all_frames(bool match_all) {
    match_all_frames_ = match_all;
  }

  std::string world_id() const {
    return world_id_;
  }
  void set_world_id(const std::string& world_id) {
    world_id_ = world_id;
  }

  bool incognito_enabled() const {
    return incognito_enabled_;
  }
  void set_incognito_enabled(bool incognito_enabled) {
    incognito_enabled_ = incognito_enabled;
  }

  bool emulate_greasemonkey() const {
    return emulate_greasemonkey_;
  }
  void set_emulate_greasemonkey(bool emulate_greasemonkey) {
    emulate_greasemonkey_ = emulate_greasemonkey;
  }

  std::string content() const {
    return contents_;
  }
  void set_content(const std::string& content) {
    contents_ = content;
  }

  void Pickle(::Pickle* pickle) const;
  void Unpickle(const ::Pickle& pickle, PickleIterator* iter);

  bool MatchesURL(const GURL& url) const;

 private:
  bool URLMatchesGlobs(const std::vector<std::string>& globs,
                       const GURL& url) const;

  static void PickleGlobs(::Pickle* pickle,
                          const std::vector<std::string>& globs);
  static void UnpickleGlobs(const ::Pickle& pickle,
                            PickleIterator* iter,
                            std::vector<std::string>& globs);
  static void PickleURLPatternSet(::Pickle* pickle,
                                  const extensions::URLPatternSet& set);
  static void UnpickleURLPatternSet(const ::Pickle& pickle,
                                    PickleIterator* iter,
                                    extensions::URLPatternSet& set);

  GURL url_;

  std::vector<std::string> include_globs_;
  std::vector<std::string> exclude_globs_;
  RunLocation run_location_;

  extensions::URLPatternSet include_pattern_set_;
  extensions::URLPatternSet exclude_pattern_set_;

  bool match_all_frames_;
  bool incognito_enabled_;
  bool emulate_greasemonkey_;

  std::string world_id_;

  std::string contents_;

  DISALLOW_COPY_AND_ASSIGN(UserScript);
};

} // namespace oxide

#endif // _OXIDE_SHARED_COMMON_USER_SCRIPT_H_
