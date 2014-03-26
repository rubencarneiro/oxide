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

#include "oxide_qt_web_frame.h"

#include <QObject>

#include "base/logging.h"

#include "qt/core/glue/oxide_qt_web_frame_adapter.h"
#include "qt/core/glue/oxide_qt_script_message_handler_adapter_p.h"

namespace oxide {
namespace qt {

namespace {
const char kAdapterKey[] = "adapter";

class AdapterData : public base::SupportsUserData::Data {
 public:
  AdapterData(WebFrameAdapter* adapter) :
      adapter(adapter) {}

  ~AdapterData() {
    delete adapterToQObject(adapter);
  }

  WebFrameAdapter* adapter;
};

}

WebFrame::~WebFrame() {}

oxide::ScriptMessageHandler* WebFrame::GetScriptMessageHandlerAt(
    size_t index) const {
  ScriptMessageHandlerAdapter* handler = GetAdapter()->message_handlers().at(index);
  return &ScriptMessageHandlerAdapterPrivate::get(handler)->handler;
}

size_t WebFrame::GetScriptMessageHandlerCount() const {
  return GetAdapter()->message_handlers().size();
}

void WebFrame::OnChildAdded(oxide::WebFrame* child) {
  QObject* child_api = adapterToQObject(static_cast<WebFrame *>(child)->GetAdapter());
  QObject* this_api = adapterToQObject(GetAdapter());

  child_api->setParent(this_api);
}

void WebFrame::OnChildRemoved(oxide::WebFrame* child) {
  QObject* child_api = adapterToQObject(static_cast<WebFrame *>(child)->GetAdapter());

  child_api->setParent(NULL);
}

void WebFrame::OnURLChanged() {
  GetAdapter()->URLChanged();
}

WebFrame::WebFrame(WebFrameAdapter* adapter,
                   content::FrameTreeNode* node,
                   oxide::WebView* view) :
    oxide::WebFrame(node, view) {
  SetUserData(kAdapterKey, new AdapterData(adapter));
  adapter->owner_ = this;
}

WebFrameAdapter* WebFrame::GetAdapter() const {
  return static_cast<AdapterData *>(GetUserData(kAdapterKey))->adapter;
}

} // namespace qt
} // namespace oxide
