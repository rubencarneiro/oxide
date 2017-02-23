// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_INPUT_INPUT_METHOD_CONTEXT_CLIENT_H_
#define _OXIDE_SHARED_BROWSER_INPUT_INPUT_METHOD_CONTEXT_CLIENT_H_

#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "third_party/WebKit/public/web/WebCompositionUnderline.h"

namespace gfx {
class Range;
}

namespace oxide {

class InputMethodContext;

class InputMethodContextClient {
 public:
  virtual ~InputMethodContextClient();

  virtual void InputPanelVisibilityChanged() = 0;

  virtual void SetComposingText(
      const base::string16& text,
      const std::vector<blink::WebCompositionUnderline>& underlines,
      const gfx::Range& selection_range) = 0;

  virtual void CommitText(const base::string16& text,
                          const gfx::Range& replacement_range) = 0;

  virtual base::string16 GetSelectionText() const = 0;

  virtual bool GetSelectedText(base::string16* text) const = 0;

 protected:
  InputMethodContextClient();

  void AttachToContext(InputMethodContext* context);
  void DetachFromContext();

  InputMethodContext* context() const { return context_; }

 private:
  InputMethodContext* context_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(InputMethodContextClient);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_INPUT_INPUT_METHOD_CONTEXT_CLIENT_H_
