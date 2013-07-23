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

#ifndef _OXIDE_COMMON_CONTENT_CLIENT_H_
#define _OXIDE_COMMON_CONTENT_CLIENT_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/common/content_client.h"

template <typename Type> struct DefaultSingletonTraits;

namespace oxide {

class ContentBrowserClient;

class ContentClient : public content::ContentClient {
 public:
  static ContentClient* GetInstance();

  ContentBrowserClient* browser();

  virtual std::string GetProduct() const OVERRIDE;
  virtual std::string GetUserAgent() const OVERRIDE;

  static bool IsBrowser();

 protected:
  // Limit default constructor access to derived classes and
  // our lazy instance initializer
  friend struct DefaultSingletonTraits<ContentClient>;
  ContentClient() :
      is_browser_(false),
      basic_startup_complete_(false) {}

 private:
  friend class ContentMainDelegate;
  friend class GlobalSettings;

  static void MaybeUpdateUserAgent();
  static void SetIsBrowser(bool is_browser);
  static void SetBasicStartupComplete(bool complete);
  static bool IsBasicStartupComplete();

  bool is_browser_;
  bool basic_startup_complete_;

  DISALLOW_COPY_AND_ASSIGN(ContentClient);
};

} // namespace oxide

#endif // _OXIDE_COMMON_CONTENT_CLIENT_H_
