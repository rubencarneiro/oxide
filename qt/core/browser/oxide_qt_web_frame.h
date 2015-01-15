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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_FRAME_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_FRAME_H_

#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"

#include "shared/browser/oxide_web_frame.h"

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class ScriptMessageRequest;
class WebFrameAdapter;

class WebFrame final : public oxide::WebFrame {
 public:
  WebFrame(WebFrameAdapter* adapter,
           oxide::WebView* view);

  bool SendMessage(const GURL& context,
                   const std::string& msg_id,
                   const std::string& args,
                   ScriptMessageRequest* req);

 private:
  friend class WebFrameAdapter;

  ~WebFrame();

  QObject* api_handle() const { return api_handle_.get(); }

  size_t GetScriptMessageHandlerCount() const final;
  const oxide::ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const final;

  void OnChildAdded(oxide::WebFrame* child) final;
  void OnChildRemoved(oxide::WebFrame* child) final;

  void URLChanged() final;

  scoped_ptr<QObject> api_handle_;
  WebFrameAdapter* adapter_;

  DISALLOW_COPY_AND_ASSIGN(WebFrame);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_FRAME_H_
