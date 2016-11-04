// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _OXIDE_MEDIA_INFO_LOADER_H_
#define _OXIDE_MEDIA_INFO_LOADER_H_

#include <memory>
#include <string>

#include "base/callback.h"
#include "base/time/time.h"
#include "content/common/content_export.h"
#include "media/blink/active_loader.h"
#include "third_party/WebKit/public/platform/WebMediaPlayer.h"
#include "third_party/WebKit/public/web/WebAssociatedURLLoaderClient.h"
#include "url/gurl.h"

namespace blink {
class WebFrame;
class WebAssociatedURLLoader;
class WebURLRequest;
}

namespace oxide {

// This class provides additional information about a media URL. Currently it
// can be used to determine if a media URL has a single security origin and
// whether the URL passes a CORS access check.
class MediaInfoLoader : private blink::WebAssociatedURLLoaderClient {
 public:
  // Status codes for start operations on MediaInfoLoader.
  enum Status {
    // The operation failed, which may have been due to:
    //   - Page navigation
    //   - Server replied 4xx/5xx
    //   - The response was invalid
    //   - Connection was terminated
    //
    // At this point you should delete the loader.
    kFailed,

    // Everything went as planned.
    kOk,
  };

  // Start loading information about the given media URL.
  // |url| - URL for the media resource to be loaded.
  // |cors_mode| - HTML media element's crossorigin attribute.
  // |ready_cb| - Called when media info has finished or failed loading.
  typedef base::Callback<void(Status)> ReadyCB;
  MediaInfoLoader(
      const GURL& url,
      blink::WebMediaPlayer::CORSMode cors_mode,
      const ReadyCB& ready_cb);
  ~MediaInfoLoader();

  // Start loading media info.
  void Start(blink::WebFrame* frame);

  // Returns true if the media resource has a single origin, false otherwise.
  // Only valid to call after the loader becomes ready.
  bool HasSingleOrigin() const;

  // Returns true if the media resource passed a CORS access control check.
  // Only valid to call after the loader becomes ready.
  bool DidPassCORSAccessCheck() const;

  void set_single_origin(bool single_origin) {
    single_origin_ = single_origin;
  }

 private:
  friend class MediaInfoLoaderTest;

  // blink::WebAssociatedURLLoaderClient implementation.
  bool willFollowRedirect(
      const blink::WebURLRequest& newRequest,
      const blink::WebURLResponse& redirectResponse);
  void didSendData(
      unsigned long long bytesSent,
      unsigned long long totalBytesToBeSent);
  void didReceiveResponse(
      const blink::WebURLResponse& response);
  void didDownloadData(
      int data_length,
      int encodedDataLength);
  void didReceiveData(
      const char* data,
      int data_length,
      int encoded_data_length);
  void didReceiveCachedMetadata(
      const char* data, int dataLength);
  void didFinishLoading(
      double finishTime);
  void didFail(
      const blink::WebURLError&);

  void DidBecomeReady(Status status);

  // Keeps track of an active WebURLLoader and associated state.
  std::unique_ptr<media::ActiveLoader> active_loader_;

  bool loader_failed_;
  GURL url_;
  blink::WebMediaPlayer::CORSMode cors_mode_;
  bool single_origin_;

  ReadyCB ready_cb_;
  base::TimeTicks start_time_;

  DISALLOW_COPY_AND_ASSIGN(MediaInfoLoader);
};

}  // namespace oxide

#endif  // OXIDE_MEDIA_INFO_LOADER_H_
