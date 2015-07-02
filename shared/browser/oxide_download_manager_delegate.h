// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_DOWNLOAD_MANAGER_DELEGATE_H_
#define _OXIDE_SHARED_BROWSER_DOWNLOAD_MANAGER_DELEGATE_H_

#include "base/macros.h"
#include "components/keyed_service/core/keyed_service.h"
#include "content/public/browser/download_manager_delegate.h"
#include "net/base/network_delegate.h"

namespace content {
class BrowserContext;
}

namespace oxide {

class DownloadManagerDelegateFactory;

class DownloadManagerDelegate : public content::DownloadManagerDelegate,
                                public KeyedService {
 public:
  static DownloadManagerDelegate* Get(content::BrowserContext* context);

 private:
  friend class DownloadManagerDelegateFactory;

  DownloadManagerDelegate(content::BrowserContext* context);
  ~DownloadManagerDelegate() override;

  // content::DownloadManagerDelegate implementation
  void GetNextId(const content::DownloadIdCallback& callback) override;
  bool DetermineDownloadTarget(
      content::DownloadItem* download,
      const content::DownloadTargetCallback& callback) override;
  bool ShouldCompleteDownload(content::DownloadItem* item,
                              const base::Closure& complete_callback) override;
  bool ShouldOpenDownload(
      content::DownloadItem* item,
      const content::DownloadOpenDelayedCallback& callback) override;

  // KeyedService implementation
  void Shutdown() override;

  content::BrowserContext* context_;

  DISALLOW_COPY_AND_ASSIGN(DownloadManagerDelegate);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_DOWNLOAD_MANAGER_DELEGATE_H_
