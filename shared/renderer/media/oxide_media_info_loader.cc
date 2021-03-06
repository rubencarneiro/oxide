// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_media_info_loader.h"

#include <utility>

#include "base/bits.h"
#include "base/callback_helpers.h"
#include "base/metrics/histogram.h"
#include "third_party/WebKit/public/platform/WebURLError.h"
#include "third_party/WebKit/public/platform/WebURLLoader.h"
#include "third_party/WebKit/public/platform/WebURLResponse.h"
#include "third_party/WebKit/public/web/WebAssociatedURLLoader.h"
#include "third_party/WebKit/public/web/WebFrame.h"

using blink::WebFrame;
using blink::WebURLError;
using blink::WebAssociatedURLLoader;
using blink::WebAssociatedURLLoaderOptions;
using blink::WebURLRequest;
using blink::WebURLResponse;

namespace oxide {

static const int kHttpOK = 200;

MediaInfoLoader::MediaInfoLoader(
    const GURL& url,
    blink::WebMediaPlayer::CORSMode cors_mode,
    const ReadyCB& ready_cb)
    : loader_failed_(false),
      url_(url),
      cors_mode_(cors_mode),
      single_origin_(true),
      ready_cb_(ready_cb) {}

MediaInfoLoader::~MediaInfoLoader() {}

void MediaInfoLoader::Start(blink::WebFrame* frame) {
  // Make sure we have not started.
  DCHECK(!ready_cb_.is_null());
  CHECK(frame);

  start_time_ = base::TimeTicks::Now();

  // Prepare the request.
  WebURLRequest request(url_);
  request.setRequestContext(WebURLRequest::RequestContextAudio);

  frame->setReferrerForRequest(request, blink::WebURL());

  std::unique_ptr<WebAssociatedURLLoader> loader;
  WebAssociatedURLLoaderOptions options;
  if (cors_mode_ == blink::WebMediaPlayer::CORSModeUnspecified) {
    options.allowCredentials = true;
    options.crossOriginRequestPolicy =
        WebAssociatedURLLoaderOptions::CrossOriginRequestPolicyAllow;
  } else {
    options.exposeAllResponseHeaders = true;
    // The author header set is empty, no preflight should go ahead.
    options.preflightPolicy = WebAssociatedURLLoaderOptions::PreventPreflight;
    options.crossOriginRequestPolicy =
        WebAssociatedURLLoaderOptions::CrossOriginRequestPolicyUseAccessControl;
    if (cors_mode_ == blink::WebMediaPlayer::CORSModeUseCredentials) {
      options.allowCredentials = true;
    }
  }
  loader.reset(frame->createAssociatedURLLoader(options));

  // Start the resource loading.
  loader->loadAsynchronously(request, this);
  active_loader_.reset(new media::ActiveLoader(std::move(loader)));
}

/////////////////////////////////////////////////////////////////////////////
// blink::WebURLLoaderClient implementation.
bool MediaInfoLoader::willFollowRedirect(
    const WebURLRequest& newRequest,
    const WebURLResponse& redirectResponse) {
  // The load may have been stopped and |ready_cb| is destroyed.
  // In this case we shouldn't do anything.
  if (ready_cb_.is_null())
    return false;

  // Only allow |single_origin_| if we haven't seen a different origin yet.
  if (single_origin_) {
    single_origin_ = url_.GetOrigin() == GURL(newRequest.url()).GetOrigin();
  }
  url_ = newRequest.url();

  return true;
}

void MediaInfoLoader::didSendData(
    unsigned long long bytes_sent,
    unsigned long long total_bytes_to_be_sent) {
  NOTIMPLEMENTED();
}

void MediaInfoLoader::didReceiveResponse(
    const WebURLResponse& response) {
  DVLOG(1) << "didReceiveResponse: HTTP/"
           << (response.httpVersion() == WebURLResponse::HTTPVersion_0_9 ? "0.9" :
               response.httpVersion() == WebURLResponse::HTTPVersion_1_0 ? "1.0" :
               response.httpVersion() == WebURLResponse::HTTPVersion_1_1 ? "1.1" :
               "Unknown")
           << " " << response.httpStatusCode();
  DCHECK(active_loader_.get());
  if (!url_.SchemeIs("http") && !url_.SchemeIs("https")) {
      DidBecomeReady(kOk);
      return;
  }
  if (response.httpStatusCode() == kHttpOK) {
    DidBecomeReady(kOk);
    return;
  }
  loader_failed_ = true;
  DidBecomeReady(kFailed);
}

void MediaInfoLoader::didReceiveData(
    const char* data,
    int data_length,
    int encoded_data_length) {
  // Ignored.
}

void MediaInfoLoader::didDownloadData(
    int dataLength,
    int encodedDataLength) {
  NOTIMPLEMENTED();
}

void MediaInfoLoader::didReceiveCachedMetadata(
    const char* data,
    int data_length) {
  NOTIMPLEMENTED();
}

void MediaInfoLoader::didFinishLoading(
    double finishTime) {
  DCHECK(active_loader_.get());
  DidBecomeReady(kOk);
}

void MediaInfoLoader::didFail(
    const WebURLError& error) {
  DVLOG(1) << "didFail: reason=" << error.reason
           << ", isCancellation=" << error.isCancellation
           << ", domain=" << error.domain.utf8().data()
           << ", localizedDescription="
           << error.localizedDescription.utf8().data();
  DCHECK(active_loader_.get());
  loader_failed_ = true;
  DidBecomeReady(kFailed);
}

bool MediaInfoLoader::HasSingleOrigin() const {
  DCHECK(ready_cb_.is_null())
      << "Must become ready before calling HasSingleOrigin()";
  return single_origin_;
}

bool MediaInfoLoader::DidPassCORSAccessCheck() const {
  DCHECK(ready_cb_.is_null())
      << "Must become ready before calling DidPassCORSAccessCheck()";
  return !loader_failed_ &&
      cors_mode_ != blink::WebMediaPlayer::CORSModeUnspecified;
}

/////////////////////////////////////////////////////////////////////////////
// Helper methods.

void MediaInfoLoader::DidBecomeReady(Status status) {
  UMA_HISTOGRAM_TIMES("Media.InfoLoadDelay",
                      base::TimeTicks::Now() - start_time_);
  active_loader_.reset();
  if (!ready_cb_.is_null()) {
    base::ResetAndReturn(&ready_cb_).Run(status);
  }
}

}  // namespace content
