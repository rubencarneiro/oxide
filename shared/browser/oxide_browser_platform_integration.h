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

#ifndef _OXIDE_SHARED_BROWSER_PLATFORM_INTEGRATION_H_
#define _OXIDE_SHARED_BROWSER_PLATFORM_INTEGRATION_H_

#include <string>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "content/public/browser/browser_thread.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"

class GURL;

namespace content {
class LocationProvider;
}

namespace ui {
class Clipboard;
}

namespace oxide {

class BrowserPlatformIntegrationObserver;
class GLContextDependent;
class MessagePump;

// An abstract interface allowing toolkit-independent code to integrate with
// toolkit-specific features
class BrowserPlatformIntegration {
 public:
  virtual ~BrowserPlatformIntegration();

  enum ApplicationState {
    // The application is about to be suspended
    APPLICATION_STATE_SUSPENDED,

    // The application is running but has no focused windows.
    // XXX: It's not clear whether anything really needs to distinguish
    //      between this and ACTIVE
    APPLICATION_STATE_INACTIVE,

    // The application is running and has a focused window
    APPLICATION_STATE_ACTIVE
  };

  // Can be called on any thread. Destruction of this class
  // must only happen once all Chromium threads have been shut down
  static BrowserPlatformIntegration* GetInstance();

  // Launch |url| in an external application. Can be called on any thread
  virtual bool LaunchURLExternally(const GURL& url);

  // Determine if there are any touch devices connected. Called on the UI
  // thread
  virtual bool IsTouchSupported();

  // Return a native display handle that can be used to create GL contexts.
  // Called on the UI thread
  virtual intptr_t GetNativeDisplay() = 0;

  // Return information about the default screen. Can be called on any thread
  virtual blink::WebScreenInfo GetDefaultScreenInfo() = 0;

  // Return the shared GL context provided by the application, if one exists.
  // This will be used for sharing resources between the webview and UI
  // compositors. Called on the UI thread
  virtual GLContextDependent* GetGLShareContext();

  // Create a MessagePump that allows Chromium events to be pumped from
  // the applications UI event loop
  virtual scoped_ptr<MessagePump> CreateUIMessagePump() = 0;

  // Create a ui::Clipboard implementation. Can return nullptr
  virtual ui::Clipboard* CreateClipboard();

  // Called on the specified browser thread
  virtual void BrowserThreadInit(content::BrowserThread::ID id);
  virtual void BrowserThreadCleanUp(content::BrowserThread::ID id);

  // Create a LocationProvider for determining location information from
  // the toolkit. Can return nullptr. Called on the geolocation thread
  virtual scoped_ptr<content::LocationProvider> CreateLocationProvider();

  // Get the current application state. Called on the UI thread
  virtual ApplicationState GetApplicationState();

  // Get the application name. Can be called on any thread
  virtual std::string GetApplicationName();

 protected:
  BrowserPlatformIntegration();

  void NotifyApplicationStateChanged();

 private:
  friend class BrowserPlatformIntegrationObserver;

  void AddObserver(BrowserPlatformIntegrationObserver* observer);
  void RemoveObserver(BrowserPlatformIntegrationObserver* observer);

  base::ObserverList<BrowserPlatformIntegrationObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(BrowserPlatformIntegration);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_PLATFORM_INTEGRATION_H_
