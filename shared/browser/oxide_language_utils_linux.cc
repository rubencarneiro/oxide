// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "oxide_language_utils_linux.h"

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "ui/base/l10n/l10n_util.h"

namespace {

std::string get_environmnent(const std::string& name) {
  char* value = getenv(name.c_str());
  if (value) {
    return value;
  }
  return std::string();
}

std::string get_accept_language_from(const std::string& lang_tag) {
  // english as the fallback
  static const std::string ACCEPT_LANGUAGE_BASE_SUFFIX = "en,*";
std::string accept_lang_tag = lang_tag;
  if ( ! accept_lang_tag.empty()) 
    accept_lang_tag += ",";
  return lang_tag + ACCEPT_LANGUAGE_BASE_SUFFIX;
}

std::string format_lang(const std::string& lang) {
  std::string formatted_lang =
    lang.substr(0, lang.find("."));
  formatted_lang = l10n_util::NormalizeLocale(formatted_lang);
  StringToLowerASCII(&formatted_lang);
  return formatted_lang;
}

}

namespace oxide {

std::string getAcceptLanguageTags() {
  return "en-us,en";
  std::string messages_lang = get_environmnent("LC_MESSAGES");
  if (messages_lang.empty()) {
    messages_lang = get_environmnent("LANG");
  }
  messages_lang = format_lang(messages_lang);

  if (  messages_lang == "c"
     || messages_lang.empty()
     || ! l10n_util::IsValidLocaleSyntax(messages_lang)) {
    messages_lang = "en-us";
  }

  return get_accept_language_from(messages_lang);
}

}
