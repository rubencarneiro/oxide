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

#ifndef _OXIDE_SHARED_BROWSER_OFF_THE_RECORD_BROWSER_CONTEXT_IMPL_H_
#define _OXIDE_SHARED_BROWSER_OFF_THE_RECORD_BROWSER_CONTEXT_IMPL_H_

#include "oxide_browser_context.h"

#include "base/basictypes.h"
#include "base/compiler_specific.h"

namespace oxide {

class BrowserContextImpl;

class OffTheRecordBrowserContextImpl FINAL : public BrowserContext {
 public:
  BrowserContext* GetOffTheRecordContext() FINAL;

  BrowserContext* GetOriginalContext() FINAL;

  base::FilePath GetPath() FINAL;

  bool IsOffTheRecord() const FINAL;

 private:
  friend class BrowserContext;
  friend class BrowserContextImpl;

  OffTheRecordBrowserContextImpl() :
      original_context_(NULL) {}
  OffTheRecordBrowserContextImpl(BrowserContextImpl* original_context) :
      original_context_(original_context) {}

  BrowserContextImpl* original_context_;

  DISALLOW_COPY_AND_ASSIGN(OffTheRecordBrowserContextImpl); 
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_OFF_THE_RECORD_BROWSER_CONTEXT_IMPL_H_ 
