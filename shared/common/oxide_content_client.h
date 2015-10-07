// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_COMMON_CONTENT_CLIENT_H_
#define _OXIDE_SHARED_COMMON_CONTENT_CLIENT_H_

#include <string>

#include "base/macros.h"
#include "content/public/common/content_client.h"

namespace content {
class ContentClientInitializer;
}

namespace oxide {

class ContentClient final : public content::ContentClient {
 public:
  ContentClient();
  ~ContentClient();

 private:
  friend class content::ContentClientInitializer; // For GetInstance

  static ContentClient* GetInstance();

  // content::ContentClient implementation
  void AddPepperPlugins(std::vector<content::PepperPluginInfo>* plugins) final;
  std::string GetUserAgent() const final;
  base::string16 GetLocalizedString(int message_id) const final;
  base::StringPiece GetDataResource(int resource_id,
                                    ui::ScaleFactor scale_factor) const final;
  base::RefCountedStaticMemory* GetDataResourceBytes(
      int resource_id) const final;
  bool ShouldOptimizeForMemoryUsage() const final;

  DISALLOW_COPY_AND_ASSIGN(ContentClient);
};

} // namespace oxide

#endif // _OXIDE_SHARED_COMMON_CONTENT_CLIENT_H_
