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

#include "oxide_download_manager_delegate.h"

#include <vector>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_item.h"
#include "content/public/browser/download_manager.h"

namespace oxide {

class DownloadManagerDelegateFactory
    : public BrowserContextKeyedServiceFactory {
 public:
  static DownloadManagerDelegateFactory* GetInstance();
  static DownloadManagerDelegate* GetForContext(
      content::BrowserContext* context);

 private:
  friend struct DefaultSingletonTraits<DownloadManagerDelegateFactory>;

  DownloadManagerDelegateFactory();
  ~DownloadManagerDelegateFactory() override;

  // BrowserContextKeyedServiceFactory methods:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;

  DISALLOW_COPY_AND_ASSIGN(DownloadManagerDelegateFactory);
};

DownloadManagerDelegateFactory::DownloadManagerDelegateFactory()
    : BrowserContextKeyedServiceFactory(
        "DownloadManagerDelegate",
        BrowserContextDependencyManager::GetInstance()) {}

DownloadManagerDelegateFactory::~DownloadManagerDelegateFactory() {}

KeyedService* DownloadManagerDelegateFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new DownloadManagerDelegate(context);
}

content::BrowserContext*
DownloadManagerDelegateFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return context;
}

// static
DownloadManagerDelegateFactory* DownloadManagerDelegateFactory::GetInstance() {
  return Singleton<DownloadManagerDelegateFactory>::get();
}

// static
DownloadManagerDelegate* DownloadManagerDelegateFactory::GetForContext(
    content::BrowserContext* context) {
  return static_cast<DownloadManagerDelegate*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

DownloadManagerDelegate::DownloadManagerDelegate(
    content::BrowserContext* context)
    : context_(context) {}

DownloadManagerDelegate::~DownloadManagerDelegate() {}

void DownloadManagerDelegate::GetNextId(
    const content::DownloadIdCallback& callback) {
  static uint32_t next_id = content::DownloadItem::kInvalidId + 1;
  callback.Run(next_id++);
}

bool DownloadManagerDelegate::DetermineDownloadTarget(
    content::DownloadItem* download,
    const content::DownloadTargetCallback& callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  if (!download->GetForcedFilePath().empty()) {
    callback.Run(download->GetForcedFilePath(),
                 content::DownloadItem::TARGET_DISPOSITION_OVERWRITE,
                 content::DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS,
                 download->GetForcedFilePath());
    return true;
  }

  return false;
}

bool DownloadManagerDelegate::ShouldCompleteDownload(
    content::DownloadItem* item,
    const base::Closure& complete_callback) {
  NOTREACHED();
  return true;
}

bool DownloadManagerDelegate::ShouldOpenDownload(
    content::DownloadItem* item,
    const content::DownloadOpenDelayedCallback& callback) {
  NOTREACHED();
  return true;
}

void DownloadManagerDelegate::Shutdown() {
  // DownloadManager::Shutdown is called in the destructor of
  // content::BrowserContext, but this is too late because the KeyedServices
  // (including |this|) have all been destroyed at this point, resulting in a
  // UAF because DownloadManager calls back in to us.
  //
  // Note, Chrome is doing the same thing here too (see
  // DownloadServiceImpl::Shutdown)
  content::BrowserContext::GetDownloadManager(context_)->Shutdown();
}

// static
DownloadManagerDelegate* DownloadManagerDelegate::Get(
    content::BrowserContext* context) {
  return DownloadManagerDelegateFactory::GetForContext(context);
}

} // namespace oxide
