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

#include "oxide_access_token_store.h"

#include "base/logging.h"

#include "oxide_io_thread.h"

namespace oxide {

AccessTokenStore::AccessTokenStore() {}

void AccessTokenStore::LoadAccessTokens(
    const LoadAccessTokensCallbackType& callback) {
  NOTIMPLEMENTED();
  AccessTokenStore::AccessTokenSet set;
  net::URLRequestContextGetter* context =
      IOThread::instance()->GetSystemURLRequestContext();
  callback.Run(set, context);
}

void AccessTokenStore::SaveAccessToken(
    const GURL& server_url, const base::string16& access_token) {
  NOTIMPLEMENTED();
}

} // namespace oxide
