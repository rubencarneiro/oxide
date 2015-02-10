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

#include "oxide_browser_context_observer.h"

#include "oxide_browser_context.h"

namespace oxide {

void BrowserContextObserver::OnBrowserContextDestruction() {
  browser_context_ = nullptr;
  BrowserContextDestroyed();
}

BrowserContextObserver::BrowserContextObserver()
    : browser_context_(nullptr) {}

BrowserContextObserver::BrowserContextObserver(BrowserContext* context)
    : browser_context_(context) {
  if (context) {
    context->AddObserver(this);
  }
}

void BrowserContextObserver::Observe(BrowserContext* context) {
  if (context == browser_context_) {
    return;
  }
  if (browser_context_) {
    browser_context_->RemoveObserver(this);
  }
  browser_context_ = context;
  if (browser_context_) {
    browser_context_->AddObserver(this);
  }
}

BrowserContextObserver::~BrowserContextObserver() {
  if (browser_context_) {
    browser_context_->RemoveObserver(this);
  }
}

} // namespace oxide
