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

#include "oxide_devtools_http_handler_delegate.h"

#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/devtools_target.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/common/url_constants.h"
#include "net/socket/tcp_listen_socket.h"
#include "net/url_request/url_request_context_getter.h"

#include "oxide_devtools_target.h"
#include "oxide_io_thread.h"
#include "oxide_web_view.h"
#include "oxide_web_view_tracker.h"

#include <sstream>

using content::DevToolsTarget;
using content::RenderViewHost;
using content::WebContents;


namespace oxide {

DevtoolsHttpHandlerDelegate::DevtoolsHttpHandlerDelegate(
      const std::string& ip,
      unsigned port,
      BrowserContext * attached_browser_context)
    : ip_(ip),
      port_(port),
      browser_context_(attached_browser_context) {

  LOG(INFO) << "Starting DevTools instance "
	    << GetLocalDevToolsJsonUrl();

  devtools_http_handler_ = content::DevToolsHttpHandler::Start(
      new net::TCPListenSocketFactory(ip, port),
      std::string(),
      this,
      base::FilePath());
}

DevtoolsHttpHandlerDelegate::~DevtoolsHttpHandlerDelegate() {
  devtools_http_handler_->Stop();
}

std::string DevtoolsHttpHandlerDelegate::GetDiscoveryPageHTML() {
  std::string devtools_json_url =
    GetLocalDevToolsJsonUrl();
  std::ostringstream oss;
  oss << "<html><head></head><body>Please use <a href=\""
      << devtools_json_url
      << "\">" << devtools_json_url << "</a>."
      << " Then use the current hostname:port and append the 'devtoolsFrontendUrl' from list."
      << "</body></html>";
  return oss.str();
}

std::string
DevtoolsHttpHandlerDelegate::GetLocalDevToolsJsonUrl() const {
  std::ostringstream oss;
  oss << "http://"
      << ip_
      << ":"
      << port_
      << "/json/list";
  return oss.str();
}

bool DevtoolsHttpHandlerDelegate::BundlesFrontendResources() {
  // We reuse the default chrome builtin webui from devtools_resources.pak
  return true;
}

base::FilePath DevtoolsHttpHandlerDelegate::GetDebugFrontendDir() {
  // We dont host the devtools resources & ui (see above).
  return base::FilePath();
}

std::string DevtoolsHttpHandlerDelegate::GetPageThumbnailData(
    const GURL& url) {
  return std::string();
}

scoped_ptr<DevToolsTarget>
DevtoolsHttpHandlerDelegate::CreateNewTarget(const GURL& url) {
  // Not supported
  return scoped_ptr<DevToolsTarget>();
}

void DevtoolsHttpHandlerDelegate::EnumerateTargets(TargetCallback callback) {
  TargetList targetList;
  WebViewTracker::WebViewList wvl =
    WebViewTracker::GetInstance()->get(browser_context_);
  if (!wvl.empty()) {
    WebViewTracker::WebViewList::iterator it = wvl.begin();
    for (; it != wvl.end(); ++it) {
      DCHECK(*it) << "Invalid WebView instance (NULL)";

      RenderViewHost * render_view_host =
	(*it)->GetWebContents()->GetRenderViewHost();
      if (render_view_host) {
	// The receiver of the target list is the owner of the content
	// See content/public/browser/devtools_http_handler_delegate.h
	targetList.push_back(
          DevtoolsTarget::CreateForRenderViewHost(render_view_host));
      }
    }
  }
  callback.Run(targetList);
}

scoped_ptr<net::StreamListenSocket>
DevtoolsHttpHandlerDelegate::CreateSocketForTethering(
    net::StreamListenSocket::Delegate* delegate,
    std::string* name) {
  // Not supported
  return scoped_ptr<net::StreamListenSocket>();
}

}
