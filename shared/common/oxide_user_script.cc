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

#include "oxide_user_script.h"

#include "base/pickle.h"
#include "base/strings/string_util.h"
#include "extensions/common/url_pattern.h"

namespace oxide {

bool UserScript::URLMatchesGlobs(const std::vector<std::string>& globs,
                                 const GURL& url) const {
  for (std::vector<std::string>::const_iterator it = globs.begin();
       it != globs.end(); ++it) {
    if (MatchPattern(url.spec(), *it)) {
      return true;
    }
  }

  return false;
}

// static
void UserScript::PickleGlobs(::Pickle* pickle,
                             const std::vector<std::string>& globs) {
  pickle->WriteUInt64(globs.size());
  for (std::vector<std::string>::const_iterator it = globs.begin();
       it != globs.end(); ++it) {
    pickle->WriteString(*it);
  }
}

// static
void UserScript::UnpickleGlobs(const ::Pickle& pickle,
                               PickleIterator* iter,
                               std::vector<std::string>& globs) {
  globs.clear();

  uint64 size = 0;
  CHECK(pickle.ReadUInt64(iter, &size));
  for (; size > 0; --size) {
    std::string glob;
    CHECK(pickle.ReadString(iter, &glob));
    globs.push_back(glob);
  }
}

// static
void UserScript::PickleURLPatternSet(::Pickle* pickle,
                                     const extensions::URLPatternSet& set) {
  pickle->WriteUInt64(set.size());
  for (extensions::URLPatternSet::const_iterator it = set.begin();
       it != set.end(); ++it) {
    pickle->WriteInt(it->valid_schemes());
    pickle->WriteString(it->GetAsString());
  }
}

// static
void UserScript::UnpickleURLPatternSet(const ::Pickle& pickle,
                                       PickleIterator* iter,
                                       extensions::URLPatternSet& set) {
  set.ClearPatterns();

  uint64 size = 0;
  CHECK(pickle.ReadUInt64(iter, &size));
  for (; size > 0; --size) {
    int valid_schemes = 0;
    CHECK(pickle.ReadInt(iter, &valid_schemes));

    std::string pattern_str;
    CHECK(pickle.ReadString(iter, &pattern_str));

    URLPattern pattern(valid_schemes);
    CHECK_EQ(pattern.Parse(pattern_str), URLPattern::PARSE_SUCCESS);

    set.AddPattern(pattern);
  }
}

UserScript::UserScript() :
    run_location_(DOCUMENT_END),
    match_all_frames_(false),
    incognito_enabled_(false),
    emulate_greasemonkey_(false) {}

void UserScript::add_exclude_glob(const std::string& glob) {
  exclude_globs_.push_back(glob);
}

void UserScript::add_include_glob(const std::string& glob) {
  include_globs_.push_back(glob);
}

void UserScript::add_include_url_pattern(const URLPattern& pattern) {
  include_pattern_set_.AddPattern(pattern);
}

void UserScript::add_exclude_url_pattern(const URLPattern& pattern) {
  exclude_pattern_set_.AddPattern(pattern);
}

void UserScript::Pickle(::Pickle* pickle) const {
  pickle->WriteInt(run_location());
  pickle->WriteBool(match_all_frames());
  pickle->WriteBool(incognito_enabled());
  pickle->WriteBool(emulate_greasemonkey());
  pickle->WriteString(world_id());

  PickleGlobs(pickle, include_globs_);
  PickleGlobs(pickle, exclude_globs_);

  PickleURLPatternSet(pickle, include_pattern_set_);
  PickleURLPatternSet(pickle, exclude_pattern_set_);

  pickle->WriteData(content().data(), content().length());
}

void UserScript::Unpickle(const ::Pickle& pickle, PickleIterator* iter) {
  int run_loc = 0;
  CHECK(pickle.ReadInt(iter, &run_loc));
  run_location_ = static_cast<RunLocation>(run_loc);
  CHECK(pickle.ReadBool(iter, &match_all_frames_));
  CHECK(pickle.ReadBool(iter, &incognito_enabled_));
  CHECK(pickle.ReadBool(iter, &emulate_greasemonkey_));
  CHECK(pickle.ReadString(iter, &world_id_));

  UnpickleGlobs(pickle, iter, include_globs_);
  UnpickleGlobs(pickle, iter, exclude_globs_);

  UnpickleURLPatternSet(pickle, iter, include_pattern_set_);
  UnpickleURLPatternSet(pickle, iter, exclude_pattern_set_);

  const char* data = NULL;
  int length = 0;
  CHECK(pickle.ReadData(iter, &data, &length));
  contents_ = std::string(data, length);
}

bool UserScript::MatchesURL(const GURL& url) const {
  if (!include_pattern_set_.is_empty()) {
    if (!include_pattern_set_.MatchesURL(url)) {
      return false;
    }
  }

  if (!exclude_pattern_set_.is_empty()) {
    if (exclude_pattern_set_.MatchesURL(url)) {
      return false;
    }
  }

  if (!include_globs_.empty()) {
    if (!URLMatchesGlobs(include_globs_, url)) {
      return false;
    }
  }

  if (!exclude_globs_.empty()) {
    if (URLMatchesGlobs(exclude_globs_, url)) {
      return false;
    }
  }

  return true;
}

} // namespace oxide
