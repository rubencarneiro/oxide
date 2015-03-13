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

#ifndef _OXIDE_SHARED_BROWSER_PLATFORM_INTEGRATION_H_
#define _OXIDE_SHARED_BROWSER_PLATFORM_INTEGRATION_H_

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "content/public/browser/browser_thread.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"

class GURL;

namespace content {
class LocationProvider;
}

namespace oxide {

class BrowserPlatformIntegrationObserver;
class GLContextAdopted;
class MessagePump;

class BrowserPlatformIntegration {
 public:
  virtual ~BrowserPlatformIntegration();

  enum ApplicationState {
    APPLICATION_STATE_INACTIVE,
    APPLICATION_STATE_ACTIVE
  };

  // Can be called on any thread. Destruction of this class
  // must only happen once all Chromium threads have been shut down
  static BrowserPlatformIntegration* GetInstance();

  // Called on the IO thread
  virtual bool LaunchURLExternally(const GURL& url);

  virtual bool IsTouchSupported();

  virtual intptr_t GetNativeDisplay() = 0;

  virtual blink::WebScreenInfo GetDefaultScreenInfo() = 0;

  virtual GLContextAdopted* GetGLShareContext();

  virtual scoped_ptr<MessagePump> CreateUIMessagePump() = 0;

  // Called on the specified browser thread
  virtual void BrowserThreadInit(content::BrowserThread::ID id);
  virtual void BrowserThreadCleanUp(content::BrowserThread::ID id);

  // Called on the geolocation thread
  virtual content::LocationProvider* CreateLocationProvider();

  virtual ApplicationState GetApplicationState();

 protected:
  BrowserPlatformIntegration();

  void NotifyApplicationStateChanged();

 private:
  friend class BrowserPlatformIntegrationObserver;

  void AddObserver(BrowserPlatformIntegrationObserver* observer);
  void RemoveObserver(BrowserPlatformIntegrationObserver* observer);

  ObserverList<BrowserPlatformIntegrationObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(BrowserPlatformIntegration);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PLATFORM_INTEGRATION_H_
