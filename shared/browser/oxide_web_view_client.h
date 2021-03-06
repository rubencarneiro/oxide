// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_WEB_VIEW_CLIENT_H_
#define _OXIDE_SHARED_BROWSER_WEB_VIEW_CLIENT_H_

#include <memory>
#include <string>

#include "base/strings/string16.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"

#include "shared/browser/oxide_script_message_target.h"
#include "shared/common/oxide_shared_export.h"

class GURL;

namespace cc {
class CompositorFrameMetadata;
}

namespace content {
struct ContextMenuParams;
class RenderFrameHost;
class WebContents;
}

namespace oxide {

class CertificateError;
class FilePicker;
class WebView;

// A class for customizing the behaviour of WebView
// TODO(chrisccoulson): Redesign ScriptMessageTarget and stop inheriting it
class OXIDE_SHARED_EXPORT WebViewClient : public ScriptMessageTarget {
 public:
  virtual ~WebViewClient();

  virtual void URLChanged();

  virtual void TitleChanged();

  virtual void FaviconChanged();

  virtual void LoadingChanged();

  virtual void LoadProgressChanged(double progress);

  virtual void LoadStarted(const GURL& validated_url);

  virtual void LoadRedirected(const GURL& url,
                              const GURL& original_url,
                              int http_status_code);

  virtual void LoadCommitted(const GURL& url,
                             bool is_error_page,
                             int http_status_code);

  virtual void LoadStopped(const GURL& validated_url);

  virtual void LoadFailed(const GURL& validated_url,
                          int error_code,
                          const std::string& error_description,
                          int http_status_code);

  virtual void LoadSucceeded(const GURL& validated_url,
                             int http_status_code);

  virtual bool AddMessageToConsole(int32_t level,
                                   const base::string16& message,
                                   int32_t line_no,
                                   const base::string16& source_id);

  // TODO(chrisccoulson): Merge with SwapCompositorFrame
  // TODO(chrisccoulson): Get rid of |old| and replace with |changed_flags|
  virtual void FrameMetadataUpdated(const cc::CompositorFrameMetadata& old);

  // XXX(chrisccoulson): WebView currently just proxies straight to this -
  //    should this get its own interface?
  virtual FilePicker* CreateFilePicker(content::RenderFrameHost* rfh);

  // TODO(chrisccoulson): Rename to BlockedContentChanged or something
  // TODO(chrisccoulson): Move content tracking to a separate class with its
  //    own delegate, as this is going to be expanded with content settings
  //    work
  virtual void ContentBlocked();

  virtual void PrepareToCloseResponseReceived(bool proceed);

  virtual void CloseRequested();

  virtual void TargetURLChanged();

  virtual void OnEditingCapabilitiesChanged();
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_CLIENT_H_
