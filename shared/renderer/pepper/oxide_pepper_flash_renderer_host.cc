// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_pepper_flash_renderer_host.h"

#include "ppapi/host/dispatch_host_message.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/proxy/host_dispatcher.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/resource_message_params.h"
#include "ppapi/proxy/serialized_structs.h"
#include "ppapi/thunk/enter.h"
#include "ppapi/thunk/ppb_image_data_api.h"
#include "content/public/renderer/renderer_ppapi_host.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/pepper_plugin_instance.h"
#include "ui/gfx/geometry/rect.h"


namespace oxide {

PepperFlashRendererHost::PepperFlashRendererHost(
    content::RendererPpapiHost* host,
    PP_Instance instance,
    PP_Resource resource)
    : ResourceHost(host->GetPpapiHost(), instance, resource),
      host_(host),
      weak_factory_(this) {}

PepperFlashRendererHost::~PepperFlashRendererHost() {
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

int32_t PepperFlashRendererHost::OnGetProxyForURL(
    ppapi::host::HostMessageContext* host_context,
    const std::string& url) {
  GURL gurl(url);
  if (!gurl.is_valid())
    return PP_ERROR_FAILED;
  std::string proxy;
  bool result = content::RenderThread::Get()->ResolveProxy(gurl, &proxy);
  if (!result)
    return PP_ERROR_FAILED;
  host_context->reply_msg = PpapiPluginMsg_Flash_GetProxyForURLReply(proxy);
  return PP_OK;
}

int32_t PepperFlashRendererHost::OnSetInstanceAlwaysOnTop(
    ppapi::host::HostMessageContext* host_context,
    bool on_top) {
  content::PepperPluginInstance* plugin_instance =
      host_->GetPluginInstance(pp_instance());
  if (plugin_instance)
    plugin_instance->SetAlwaysOnTop(on_top);
  // Since no reply is sent for this message, it doesn't make sense to return an
  // error.
  return PP_OK;
}

int32_t PepperFlashRendererHost::OnDrawGlyphs(
    ppapi::host::HostMessageContext* host_context,
    ppapi::proxy::PPBFlash_DrawGlyphs_Params params) {
  if (params.glyph_indices.size() != params.glyph_advances.size() ||
      params.glyph_indices.empty())
    return PP_ERROR_FAILED;

  return PP_OK;
}

int32_t PepperFlashRendererHost::OnNavigate(
    ppapi::host::HostMessageContext* host_context,
    const ppapi::URLRequestInfoData& data,
    const std::string& target,
    bool from_user_action) {
  return PP_OK_COMPLETIONPENDING;
}

int32_t PepperFlashRendererHost::OnIsRectTopmost(
    ppapi::host::HostMessageContext* host_context,
    const PP_Rect& rect) {

  content::PepperPluginInstance* plugin_instance =
      host_->GetPluginInstance(pp_instance());

  if (plugin_instance &&
      plugin_instance->IsRectTopmost(gfx::Rect(
          rect.point.x, rect.point.y, rect.size.width, rect.size.height)))
    return PP_OK;

  return PP_ERROR_FAILED;
}

int32_t PepperFlashRendererHost::OnInvokePrinting(
    ppapi::host::HostMessageContext* host_context) {

  return PP_OK;
}

} // oxide
