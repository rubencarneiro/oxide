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

#ifndef _OXIDE_SHARED_ACCESS_TOKEN_STORE_H_
#define _OXIDE_SHARED_ACCESS_TOKEN_STORE_H_

#include "content/public/browser/access_token_store.h"

namespace oxide {

class AccessTokenStore : public content::AccessTokenStore
{
 public:
  AccessTokenStore();

  virtual void LoadAccessTokens(
      const LoadAccessTokensCallbackType& callback) OVERRIDE;
  virtual void SaveAccessToken(
      const GURL& server_url, const base::string16& access_token) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(AccessTokenStore);
};

} // namespace oxide

#endif // _OXIDE_SHARED_ACCESS_TOKEN_STORE_H_
