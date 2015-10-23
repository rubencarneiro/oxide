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

#ifndef _OXIDE_SHARED_BROWSER_RENDER_OBJECT_ID_H_
#define _OXIDE_SHARED_BROWSER_RENDER_OBJECT_ID_H_

#include "base/logging.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_widget_host.h"
#include "ipc/ipc_message.h"

namespace oxide {

// Helper class that allows a reference to a RenderFrameHost or
// RenderWidgetHost to be stored safely without having to implement
// WebContentsObserver (if you don't need notification of the object being
// deleted) or without risking a dangling pointer
template <class T>
class ID {
 public:
  ID() : render_process_id_(MSG_ROUTING_NONE),
         render_object_id_(MSG_ROUTING_NONE) {}

  ID(T* object)
      : render_process_id_(object->GetProcess()->GetID()),
        render_object_id_(object->GetRoutingID()) {}

  T* ToInstance() const {
    DCHECK(IsValid());
    return T::FromID(render_process_id_, render_object_id_);
  }

  bool IsValid() const {
    return render_process_id_ != MSG_ROUTING_NONE &&
           render_object_id_ != MSG_ROUTING_NONE;
  }

  bool operator==(const ID<T>& other) const {
    return render_process_id_ == other.render_process_id_ &&
           render_object_id_ == other.render_object_id_;
  }

  bool operator!=(const ID<T>& other) const {
    return !(*this == other);
  }

  // Allows this to be used as a key in std::map
  bool operator<(const ID<T>& other) const {
    if (render_process_id_ < other.render_process_id_) {
      return true;
    }

    if (render_process_id_ != other.render_process_id_) {
      return false;
    }

    if (render_object_id_ < other.render_object_id_) {
      return true;
    }

    return false;
  }

 private:
  int render_process_id_;
  int render_object_id_;
};

typedef ID<content::RenderFrameHost> RenderFrameHostID;
typedef ID<content::RenderWidgetHost> RenderWidgetHostID;

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_OBJECT_ID_H_
