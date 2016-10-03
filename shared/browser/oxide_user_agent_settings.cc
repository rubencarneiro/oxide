// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

#include "oxide_user_agent_settings.h"

#include <libintl.h>

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/strings/stringprintf.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/resource_context.h"
#include "content/public/common/user_agent.h"

#include "shared/common/chrome_version.h"
#include "shared/common/oxide_messages.h"

#include "oxide_browser_context.h"
#include "oxide_browser_context_delegate.h"
#include "oxide_user_agent_settings_observer.h"

namespace oxide {

namespace {
const char kDefaultAcceptLanguage[] = "en-us,en";
}

class UserAgentSettingsFactory : public BrowserContextKeyedServiceFactory {
 public:
  static UserAgentSettingsFactory* GetInstance();
  static UserAgentSettings* GetForContext(content::BrowserContext* context);

 private:
  friend struct base::DefaultSingletonTraits<UserAgentSettingsFactory>;

  UserAgentSettingsFactory();
  ~UserAgentSettingsFactory() override;

  // BrowserContextKeyedServiceFactory methods:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;

  DISALLOW_COPY_AND_ASSIGN(UserAgentSettingsFactory);
};

UserAgentSettingsFactory::UserAgentSettingsFactory()
    : BrowserContextKeyedServiceFactory(
        "UserAgentSettings",
        BrowserContextDependencyManager::GetInstance()) {}

UserAgentSettingsFactory::~UserAgentSettingsFactory() {}

KeyedService* UserAgentSettingsFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new UserAgentSettings(BrowserContext::FromContent(context));
}

content::BrowserContext* UserAgentSettingsFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return BrowserContext::FromContent(context)->GetOriginalContext();
}

// static
UserAgentSettingsFactory* UserAgentSettingsFactory::GetInstance() {
  return base::Singleton<UserAgentSettingsFactory>::get();
}

// static
UserAgentSettings* UserAgentSettingsFactory::GetForContext(
    content::BrowserContext* context) {
  return static_cast<UserAgentSettings*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

void UserAgentSettingsIOData::SetUserAgentOverrides(
    const std::vector<UserAgentOverrideSet::Entry>& overrides) {
  user_agent_override_set_.SetOverrides(overrides);
}

UserAgentSettingsIOData::UserAgentSettingsIOData(BrowserContextIOData* context)
    : context_(context),
      popup_blocker_enabled_(true),
      do_not_track_(false) {
  // TRANSLATORS: AcceptLanguage is a special token that should not be
  // translated as such. The expected value is a comma-separated list of
  // language codes in order of decreasing preference (e.g. "es-ES,es,en,*").
  // See https://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.4.
  accept_langs_ = dgettext(OXIDE_GETTEXT_DOMAIN, "AcceptLanguage");
  if (accept_langs_ == "AcceptLanguage") {
    accept_langs_ = kDefaultAcceptLanguage;
  }
}

UserAgentSettingsIOData::~UserAgentSettingsIOData() {}

std::string UserAgentSettingsIOData::GetUserAgent() const {
  base::AutoLock lock(lock_);
  return user_agent_;
}

std::string UserAgentSettingsIOData::GetAcceptLangs() const {
  base::AutoLock lock(lock_);
  return accept_langs_;
}

std::string UserAgentSettingsIOData::GetUserAgentForURL(const GURL& url) {
  std::string override_ua = GetUserAgentOverrideForURL(url);
  if (!override_ua.empty()) {
    return override_ua;
  }

  return GetUserAgent();
}

std::string UserAgentSettingsIOData::GetUserAgentOverrideForURL(
    const GURL& url) {
  return user_agent_override_set_.GetOverrideForURL(url);
}

bool UserAgentSettingsIOData::IsPopupBlockerEnabled() const {
  base::AutoLock lock(lock_);
  return popup_blocker_enabled_;
}

bool UserAgentSettingsIOData::GetDoNotTrack() const {
  base::AutoLock lock(lock_);
  return do_not_track_;
}

UserAgentSettings::UserAgentSettings(BrowserContext* context)
    : context_(context),
      product_(base::StringPrintf("Chrome/%s", CHROME_VERSION_STRING)),
      user_agent_string_is_default_(true),
      legacy_user_agent_override_enabled_(false) {
  UserAgentSettingsIOData* io_data =
      context_->GetIOData()->GetUserAgentSettings();
  io_data->user_agent_ = content::BuildUserAgentFromProduct(product_);
}

UserAgentSettings::~UserAgentSettings() {}

UserAgentSettings::HostSet UserAgentSettings::GetHostSet() const {
  HostSet hosts;

  for (content::RenderProcessHost::iterator it =
          content::RenderProcessHost::AllHostsIterator();
       !it.IsAtEnd(); it.Advance()) {
    content::RenderProcessHost* host = it.GetCurrentValue();
    BrowserContext* other =
        BrowserContext::FromContent(host->GetBrowserContext());
    if (context_->IsSameContext(other)) {
      hosts.insert(host);
    }
  }

  return hosts;
}

void UserAgentSettings::UpdateUserAgentForHost(
    content::RenderProcessHost* host) {
  host->Send(new OxideMsg_SetUserAgent(GetUserAgent()));
}

void UserAgentSettings::UpdateUserAgentOverridesForHost(
    content::RenderProcessHost* host) {
  host->Send(new OxideMsg_UpdateUserAgentOverrides(user_agent_overrides_));
}

void UserAgentSettings::UpdateLegacyUserAgentOverrideEnabledForHost(
    content::RenderProcessHost* host) {
  host->Send(
      new OxideMsg_SetLegacyUserAgentOverrideEnabled(
        legacy_user_agent_override_enabled_));
}

void UserAgentSettings::AddObserver(UserAgentSettingsObserver* observer) {
  DCHECK(CalledOnValidThread());
  observers_.AddObserver(observer);
}

void UserAgentSettings::RemoveObserver(UserAgentSettingsObserver* observer) {
  DCHECK(CalledOnValidThread());
  observers_.RemoveObserver(observer);
}

// static
UserAgentSettings* UserAgentSettings::Get(content::BrowserContext* context) {
  return UserAgentSettingsFactory::GetForContext(context);
}

// static
BrowserContextKeyedServiceFactory* UserAgentSettings::GetFactory() {
  return UserAgentSettingsFactory::GetInstance();
}

std::string UserAgentSettings::GetUserAgent() const {
  DCHECK(CalledOnValidThread());

  return context_->GetIOData()->GetUserAgentSettings()->user_agent_;
}

void UserAgentSettings::SetUserAgent(const std::string& user_agent) {
  DCHECK(CalledOnValidThread());

  {
    UserAgentSettingsIOData* io_data =
        context_->GetIOData()->GetUserAgentSettings();
    base::AutoLock lock(io_data->lock_);
    io_data->user_agent_ = user_agent.empty() ?
        content::BuildUserAgentFromProduct(product_) :
        user_agent;
  }

  user_agent_string_is_default_ = user_agent.empty();

  std::set<content::RenderProcessHost*> hosts = GetHostSet();
  for (auto host : hosts) {
    UpdateUserAgentForHost(host);
  }
}

std::string UserAgentSettings::GetProduct() const {
  DCHECK(CalledOnValidThread());
  return product_;
}

void UserAgentSettings::SetProduct(const std::string& product) {
  DCHECK(CalledOnValidThread());

  product_ = product.empty() ?
      base::StringPrintf("Chrome/%s", CHROME_VERSION_STRING) :
      product;
  if (user_agent_string_is_default_) {
    SetUserAgent(std::string());
  }
}

std::string UserAgentSettings::GetAcceptLangs() const {
  DCHECK(CalledOnValidThread());

  return context_->GetIOData()->GetUserAgentSettings()->accept_langs_;
}

void UserAgentSettings::SetAcceptLangs(const std::string& accept_langs) {
  DCHECK(CalledOnValidThread());

  UserAgentSettingsIOData* io_data =
      context_->GetIOData()->GetUserAgentSettings();
  base::AutoLock lock(io_data->lock_);
  io_data->accept_langs_ = accept_langs;
}

std::vector<UserAgentSettings::UserAgentOverride>
UserAgentSettings::GetUserAgentOverrides() const {
  DCHECK(CalledOnValidThread());
  return user_agent_overrides_;
}

void UserAgentSettings::SetUserAgentOverrides(
    const std::vector<UserAgentOverride>& overrides) {
  DCHECK(CalledOnValidThread());

  user_agent_overrides_ = overrides;
 
  UserAgentSettingsIOData* io_data =
      context_->GetIOData()->GetUserAgentSettings();
  io_data->user_agent_override_set_.SetOverrides(overrides);

  HostSet hosts = GetHostSet();
  for (auto host : hosts) {
    UpdateUserAgentOverridesForHost(host);
  }
}

std::string UserAgentSettings::GetUserAgentForURL(const GURL& url) {
  DCHECK(CalledOnValidThread());

  UserAgentSettingsIOData* io_data =
      context_->GetIOData()->GetUserAgentSettings();

  std::string override_ua =
      io_data->user_agent_override_set_.GetOverrideForURL(url);
  if (!override_ua.empty()) {
    return override_ua;
  }

  return GetUserAgent();
}

void UserAgentSettings::RenderProcessCreated(
    content::RenderProcessHost* process) {
  UpdateUserAgentForHost(process);
  UpdateUserAgentOverridesForHost(process);
  UpdateLegacyUserAgentOverrideEnabledForHost(process);
}

void UserAgentSettings::SetLegacyUserAgentOverrideEnabled(bool enabled) {
  if (enabled == legacy_user_agent_override_enabled_) {
    return;
  }

  legacy_user_agent_override_enabled_ = enabled;

  std::set<content::RenderProcessHost*> hosts = GetHostSet();
  for (auto host : hosts) {
    UpdateLegacyUserAgentOverrideEnabledForHost(host);
  }
}

bool UserAgentSettings::IsPopupBlockerEnabled() const {
  DCHECK(CalledOnValidThread());

  return context_->GetIOData()->GetUserAgentSettings()->popup_blocker_enabled_;
}

void UserAgentSettings::SetIsPopupBlockerEnabled(bool enabled) {
  DCHECK(CalledOnValidThread());

  UserAgentSettingsIOData* io_data =
      context_->GetIOData()->GetUserAgentSettings();
  base::AutoLock lock(io_data->lock_);
  io_data->popup_blocker_enabled_ = enabled;

  FOR_EACH_OBSERVER(UserAgentSettingsObserver,
                    observers_,
                    NotifyPopupBlockerEnabledChanged());
}

bool UserAgentSettings::GetDoNotTrack() const {
  DCHECK(CalledOnValidThread());

  return context_->GetIOData()->GetUserAgentSettings()->do_not_track_;
}

void UserAgentSettings::SetDoNotTrack(bool dnt) {
  DCHECK(CalledOnValidThread());

  UserAgentSettingsIOData* io_data =
      context_->GetIOData()->GetUserAgentSettings();
  base::AutoLock lock(io_data->lock_);
  io_data->do_not_track_ = dnt;

  FOR_EACH_OBSERVER(UserAgentSettingsObserver,
                    observers_,
                    NotifyDoNotTrackChanged());
}

} // namespace oxide
