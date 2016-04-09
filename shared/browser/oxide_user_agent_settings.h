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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

#ifndef _OXIDE_SHARED_BROWSER_USER_AGENT_SETTINGS_H_
#define _OXIDE_SHARED_BROWSER_USER_AGENT_SETTINGS_H_

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/macros.h"
#include "base/synchronization/lock.h"
#include "base/threading/non_thread_safe.h"
#include "components/keyed_service/core/keyed_service.h"

#include "shared/common/oxide_shared_export.h"
#include "shared/common/oxide_user_agent_override_set.h"

class BrowserContextKeyedServiceFactory;

namespace content {
class BrowserContext;
class RenderProcessHost;
}

namespace oxide {

class BrowserContext;
class BrowserContextIOData;
class UserAgentSettings;
class UserAgentSettingsFactory;

// IO thread part of UserAgentSettings (see below) - accessed from
// BrowserContextIOData
class UserAgentSettingsIOData {
 public:
  UserAgentSettingsIOData(BrowserContextIOData* context);
  ~UserAgentSettingsIOData();

  // Get the default user agent string
  std::string GetUserAgent() const;

  // Get the comma-separated list of languages for the HTTP Accept-Language
  // header
  std::string GetAcceptLangs() const;

  // Get the user agent string for the specified |url|. If no override exists,
  // this will return the same as GetUserAgent
  std::string GetUserAgentForURL(const GURL& url);

  // Get the override user agent string for the specified |url|
  std::string GetUserAgentOverrideForURL(const GURL& url);

 private:
  friend class UserAgentSettings;

  void SetUserAgentOverrides(
      const std::vector<UserAgentOverrideSet::Entry>& overrides);

  BrowserContextIOData* context_;

  // Used to protect user_agent_ and accept_langs_ when being modified on
  // the UI thread
  mutable base::Lock lock_;

  std::string user_agent_;

  std::string accept_langs_;

  UserAgentOverrideSet user_agent_override_set_;

  DISALLOW_COPY_AND_ASSIGN(UserAgentSettingsIOData);
};

// Per-BrowserContext user-agent settings. This must only be used on the
// UI thread
class OXIDE_SHARED_EXPORT UserAgentSettings : public KeyedService,
                                              public base::NonThreadSafe {
 public:
  static UserAgentSettings* Get(content::BrowserContext* context);

  static BrowserContextKeyedServiceFactory* GetFactory();

  // Get the default user agent string
  std::string GetUserAgent() const;

  // Set the default user agent string. An empty string will restore it to
  // the built-in default
  void SetUserAgent(const std::string& user_agent);

  // Get the product name
  std::string GetProduct() const;

  // Set the product name. This is added to the built-in default user agent
  // string
  void SetProduct(const std::string& product);

  // Get the comma-separated list of languages for the HTTP Accept-Language
  // header
  std::string GetAcceptLangs() const;

  // Set the comma-separated list of languages for the HTTP Accept-Language
  // header
  void SetAcceptLangs(const std::string& accept_langs);

  typedef std::pair<std::string, std::string> UserAgentOverride;

  // Get the list of per-URL user agent string overrides
  std::vector<UserAgentOverride> GetUserAgentOverrides() const;

  // Set the list of per-URL user agent string overrides. Each entry
  // consists of a string that can be compiled as a regular expression for
  // URL matching, and an override UA string
  void SetUserAgentOverrides(const std::vector<UserAgentOverride>& overrides);

  // Get the user agent string for the specified |url|. If no override exists,
  // this will return the same as GetUserAgent
  std::string GetUserAgentForURL(const GURL& url);

  // Initialize |process|
  void RenderProcessCreated(content::RenderProcessHost* process);

  // Whether to enable the legacy user agent override mechanism, which
  // works by proxying lookups from the renderer to BrowserContextDelegate
  // via synchronous IPC
  void SetLegacyUserAgentOverrideEnabled(bool enabled);

 private:
  friend class UserAgentSettingsFactory;
  typedef std::set<content::RenderProcessHost*> HostSet;

  UserAgentSettings(BrowserContext* context);
  ~UserAgentSettings() override;

  HostSet GetHostSet() const;
  void UpdateUserAgentForHost(content::RenderProcessHost* host);
  void UpdateUserAgentOverridesForHost(content::RenderProcessHost* host);
  void UpdateLegacyUserAgentOverrideEnabledForHost(
      content::RenderProcessHost* host);

  BrowserContext* context_;

  std::string product_;

  bool user_agent_string_is_default_;

  std::vector<UserAgentOverride> user_agent_overrides_;

  UserAgentOverrideSet user_agent_override_set_;

  bool legacy_user_agent_override_enabled_;

  DISALLOW_COPY_AND_ASSIGN(UserAgentSettings);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_USER_AGENT_SETTINGS_H_
