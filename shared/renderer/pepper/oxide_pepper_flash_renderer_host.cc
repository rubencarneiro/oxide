// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

// Some of this file is based on
// chrome/renderer/pepper/pepper_flash_renderer_host.cc from Chromium:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_pepper_flash_renderer_host.h"

#include "base/strings/string_util.h"
#include "content/public/renderer/renderer_ppapi_host.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/pepper_plugin_instance.h"
#include "net/http/http_util.h"
#include "ppapi/host/dispatch_host_message.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/proxy/host_dispatcher.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/resource_message_params.h"
#include "ppapi/proxy/serialized_structs.h"
#include "ppapi/thunk/enter.h"
#include "ppapi/thunk/ppb_image_data_api.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkFontStyle.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkScalar.h"
#include "third_party/skia/include/core/SkTypeface.h"
#include "ui/gfx/geometry/rect.h"

namespace oxide {

namespace {

bool IsSimpleHeader(const std::string& lower_case_header_name,
                    const std::string& header_value) {
  if (lower_case_header_name == "accept" ||
      lower_case_header_name == "accept-language" ||
      lower_case_header_name == "content-language") {
    return true;
  }

  if (lower_case_header_name == "content-type") {
    std::string lower_case_mime_type;
    std::string lower_case_charset;
    bool had_charset = false;
    net::HttpUtil::ParseContentType(header_value,
                                    &lower_case_mime_type,
                                    &lower_case_charset,
                                    &had_charset,
                                    NULL);
    return lower_case_mime_type == "application/x-www-form-urlencoded" ||
           lower_case_mime_type == "multipart/form-data" ||
           lower_case_mime_type == "text/plain";
  }

  return false;
}

}

int32_t PepperFlashRendererHost::OnGetProxyForURL(
    ppapi::host::HostMessageContext* host_context,
    const std::string& url) {
  GURL gurl(url);
  if (!gurl.is_valid()) {
    return PP_ERROR_FAILED;
  }

  std::string proxy;
  bool result = content::RenderThread::Get()->ResolveProxy(gurl, &proxy);
  if (!result) {
    return PP_ERROR_FAILED;
  }

  host_context->reply_msg = PpapiPluginMsg_Flash_GetProxyForURLReply(proxy);
  return PP_OK;
}

int32_t PepperFlashRendererHost::OnSetInstanceAlwaysOnTop(
    ppapi::host::HostMessageContext* host_context,
    bool on_top) {
  content::PepperPluginInstance* plugin_instance =
      host_->GetPluginInstance(pp_instance());
  if (plugin_instance) {
    plugin_instance->SetAlwaysOnTop(on_top);
  }

  // Since no reply is sent for this message, it doesn't make sense to return an
  // error.
  return PP_OK;
}

int32_t PepperFlashRendererHost::OnDrawGlyphs(
    ppapi::host::HostMessageContext* host_context,
    ppapi::proxy::PPBFlash_DrawGlyphs_Params params) {
  if (params.glyph_indices.size() != params.glyph_advances.size() ||
      params.glyph_indices.empty()) {
    return PP_ERROR_FAILED;
  }

  // Set up the typeface.
  int style = SkTypeface::kNormal;
  if (static_cast<PP_BrowserFont_Trusted_Weight>(params.font_desc.weight) >=
      PP_BROWSERFONT_TRUSTED_WEIGHT_BOLD) {
    style |= SkTypeface::kBold;
  }
  if (params.font_desc.italic) {
    style |= SkTypeface::kItalic;
  }
  sk_sp<SkTypeface> typeface =
      SkTypeface::MakeFromName(params.font_desc.face.c_str(),
                               SkFontStyle::FromOldStyle(style));
  if (!typeface) {
    return PP_ERROR_FAILED;
  }

  ppapi::thunk::EnterResourceNoLock<ppapi::thunk::PPB_ImageData_API> enter(
      params.image_data.host_resource(),
      true);
  if (enter.failed()) {
    return PP_ERROR_FAILED;
  }

  // Set up the canvas.
  ppapi::thunk::PPB_ImageData_API* image =
      static_cast<ppapi::thunk::PPB_ImageData_API*>(enter.object());
  SkCanvas* canvas = image->GetCanvas();
  bool needs_unmapping = false;
  if (!canvas) {
    needs_unmapping = true;
    image->Map();
    canvas = image->GetCanvas();
    if (!canvas) {
      return PP_ERROR_FAILED;  // Failure mapping.
    }
  }

  SkAutoCanvasRestore acr(canvas, true);

  // Clip is applied in pixels before the transform.
  SkRect clip_rect = {
      SkIntToScalar(params.clip.point.x), SkIntToScalar(params.clip.point.y),
      SkIntToScalar(params.clip.point.x + params.clip.size.width),
      SkIntToScalar(params.clip.point.y + params.clip.size.height)};
  canvas->clipRect(clip_rect);

  // Convert & set the matrix.
  SkMatrix matrix;
  matrix.set(SkMatrix::kMScaleX, SkFloatToScalar(params.transformation[0][0]));
  matrix.set(SkMatrix::kMSkewX, SkFloatToScalar(params.transformation[0][1]));
  matrix.set(SkMatrix::kMTransX, SkFloatToScalar(params.transformation[0][2]));
  matrix.set(SkMatrix::kMSkewY, SkFloatToScalar(params.transformation[1][0]));
  matrix.set(SkMatrix::kMScaleY, SkFloatToScalar(params.transformation[1][1]));
  matrix.set(SkMatrix::kMTransY, SkFloatToScalar(params.transformation[1][2]));
  matrix.set(SkMatrix::kMPersp0, SkFloatToScalar(params.transformation[2][0]));
  matrix.set(SkMatrix::kMPersp1, SkFloatToScalar(params.transformation[2][1]));
  matrix.set(SkMatrix::kMPersp2, SkFloatToScalar(params.transformation[2][2]));
  canvas->concat(matrix);

  SkPaint paint;
  paint.setColor(params.color);
  paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
  paint.setAntiAlias(true);
  paint.setHinting(SkPaint::kFull_Hinting);
  paint.setTextSize(SkIntToScalar(params.font_desc.size));
  paint.setTypeface(typeface);  // Takes a ref and manages lifetime.
  if (params.allow_subpixel_aa) {
    paint.setSubpixelText(true);
    paint.setLCDRenderText(true);
  }

  SkScalar x = SkIntToScalar(params.position.x);
  SkScalar y = SkIntToScalar(params.position.y);

  // Build up the skia advances.
  size_t glyph_count = params.glyph_indices.size();
  if (glyph_count) {
    std::vector<SkPoint> sk_positions(glyph_count);
    for (uint32_t i = 0; i < glyph_count; i++) {
      sk_positions[i].set(x, y);
      x += SkFloatToScalar(params.glyph_advances[i].x);
      y += SkFloatToScalar(params.glyph_advances[i].y);
    }

    canvas->drawPosText(
        &params.glyph_indices[0], glyph_count * 2, &sk_positions[0], paint);
  }

  if (needs_unmapping) {
    image->Unmap();
  }

  return PP_OK;
}

// CAUTION: This code is subtle because Navigate is a sync call which may
// cause re-entrancy or cause the instance to be destroyed. If the instance
// is destroyed we need to ensure that we respond to all outstanding sync
// messages so that the plugin process does not remain blocked.
int32_t PepperFlashRendererHost::OnNavigate(
    ppapi::host::HostMessageContext* host_context,
    const ppapi::URLRequestInfoData& data,
    const std::string& target,
    bool from_user_action) {
  // If our PepperPluginInstance is already destroyed, just return a failure.
  content::PepperPluginInstance* plugin_instance =
      host_->GetPluginInstance(pp_instance());
  if (!plugin_instance) {
    return PP_ERROR_FAILED;
  }

  net::HttpUtil::HeadersIterator header_iter(
      data.headers.begin(), data.headers.end(), "\n\r");
  bool rejected = false;
  while (header_iter.GetNext()) {
    std::string lower_case_header_name =
        base::ToLowerASCII(header_iter.name());
    if (!IsSimpleHeader(lower_case_header_name, header_iter.values())) {
      rejected = true;
      break;
    }
  }

  if (rejected) {
    return PP_ERROR_NOACCESS;
  }

  // Navigate may call into Javascript (e.g. with a "javascript:" URL),
  // or do things like navigate away from the page, either one of which will
  // need to re-enter into the plugin. It is safe, because it is essentially
  // equivalent to NPN_GetURL, where Flash would expect re-entrancy.
  ppapi::proxy::HostDispatcher* host_dispatcher =
      ppapi::proxy::HostDispatcher::GetForInstance(pp_instance());
  host_dispatcher->set_allow_plugin_reentrancy();

  // Grab a weak pointer to ourselves on the stack so we can check if we are
  // still alive.
  base::WeakPtr<PepperFlashRendererHost> weak_ptr = weak_factory_.GetWeakPtr();
  // Keep track of reply contexts in case we are destroyed during a Navigate
  // call. Even if we are destroyed, we still need to send these replies to
  // unblock the plugin process.
  navigate_replies_.push_back(host_context->MakeReplyMessageContext());
  plugin_instance->Navigate(data, target.c_str(), from_user_action);
  // This object might have been destroyed by this point. If it is destroyed
  // the reply will be sent in the destructor. Otherwise send the reply here.
  if (weak_ptr.get()) {
    SendReply(navigate_replies_.back(), IPC::Message());
    navigate_replies_.pop_back();
  }

  // Return PP_OK_COMPLETIONPENDING so that no reply is automatically sent.
  return PP_OK_COMPLETIONPENDING;
}

int32_t PepperFlashRendererHost::OnIsRectTopmost(
    ppapi::host::HostMessageContext* host_context,
    const PP_Rect& rect) {
  content::PepperPluginInstance* plugin_instance =
      host_->GetPluginInstance(pp_instance());

  if (plugin_instance &&
      plugin_instance->IsRectTopmost(gfx::Rect(
          rect.point.x, rect.point.y, rect.size.width, rect.size.height))) {
    return PP_OK;
  }

  return PP_ERROR_FAILED;
}

int32_t PepperFlashRendererHost::OnInvokePrinting(
    ppapi::host::HostMessageContext* host_context) {

  return PP_OK;
}

int32_t PepperFlashRendererHost::OnResourceMessageReceived(
    const IPC::Message& msg,
    ppapi::host::HostMessageContext* context) {
  PPAPI_BEGIN_MESSAGE_MAP(PepperFlashRendererHost, msg)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_Flash_GetProxyForURL,
                                      OnGetProxyForURL)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_Flash_SetInstanceAlwaysOnTop,
                                      OnSetInstanceAlwaysOnTop)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_Flash_DrawGlyphs,
                                      OnDrawGlyphs)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_Flash_Navigate, OnNavigate)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_Flash_IsRectTopmost,
                                      OnIsRectTopmost)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL_0(PpapiHostMsg_Flash_InvokePrinting,
                                        OnInvokePrinting)
  PPAPI_END_MESSAGE_MAP()
  return PP_ERROR_FAILED;
}

PepperFlashRendererHost::PepperFlashRendererHost(
    content::RendererPpapiHost* host,
    PP_Instance instance,
    PP_Resource resource)
    : ResourceHost(host->GetPpapiHost(), instance, resource),
      host_(host),
      weak_factory_(this) {}

PepperFlashRendererHost::~PepperFlashRendererHost() {
  // This object may be destroyed in the middle of a sync message. If that is
  // the case, make sure we respond to all the pending navigate calls.
  std::vector<ppapi::host::ReplyMessageContext>::reverse_iterator it;
  for (it = navigate_replies_.rbegin(); it != navigate_replies_.rend(); ++it) {
    SendReply(*it, IPC::Message());
  }
}

} // oxide
