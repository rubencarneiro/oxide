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

#include "oxide_isolated_world_map.h"

#include <map>

#include "base/lazy_instance.h"
#include "base/logging.h"

#include "shared/common/oxide_constants.h"

namespace oxide {

namespace {

const GURL g_oxide_main_world_private_url("oxide-private://main-world-private");

int g_next_isolated_world_id = 1;

typedef std::map<GURL, int> WorldIDMap;
typedef WorldIDMap::iterator WorldIDMapIterator;
base::LazyInstance<WorldIDMap> g_isolated_world_map =
    LAZY_INSTANCE_INITIALIZER;

} // namespace

// static
int IsolatedWorldMap::IDFromURL(const GURL& url) {
  CHECK(url.is_valid());

  if (url == g_oxide_main_world_private_url)
    return kMainWorldId;

  WorldIDMapIterator it = g_isolated_world_map.Get().find(url);
  if (it != g_isolated_world_map.Get().end()) {
    return it->second;
  }

  int new_id = g_next_isolated_world_id++;
  g_isolated_world_map.Get()[url] = new_id;

  return new_id;
}

// static
GURL IsolatedWorldMap::URLFromID(int id) {
  if (id == kMainWorldId)
    // TODO put as a constant
    return g_oxide_main_world_private_url;

  for (WorldIDMapIterator it = g_isolated_world_map.Get().begin();
       it != g_isolated_world_map.Get().end(); ++it) {
    if (it->second == id) {
      return it->first;
    }
  }

  DCHECK(0) << "Invalid world ID";
  return GURL();
}

} // namespace oxide
