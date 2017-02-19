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

#include "oxide_clipboard_dummy_impl.h"

#include "base/logging.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace oxide {

void ClipboardDummyImpl::OnPreShutdown() {}

uint64_t ClipboardDummyImpl::GetSequenceNumber(ui::ClipboardType type) const {
  DCHECK(CalledOnValidThread());
  return 0;
}

bool ClipboardDummyImpl::IsFormatAvailable(const FormatType& format,
                                           ui::ClipboardType type) const {
  DCHECK(CalledOnValidThread());
  return false;
}

void ClipboardDummyImpl::Clear(ui::ClipboardType type) {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::ReadAvailableTypes(ui::ClipboardType type,
                                            std::vector<base::string16>* types,
                                            bool* contains_filenames) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::ReadText(ui::ClipboardType type,
                                  base::string16* result) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::ReadAsciiText(ui::ClipboardType type,
                                       std::string* result) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::ReadHTML(ui::ClipboardType type,
                                  base::string16* markup,
                                  std::string* src_url,
                                  uint32_t* fragment_start,
                                  uint32_t* fragment_end) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::ReadRTF(ui::ClipboardType type,
                                 std::string* result) const {
  DCHECK(CalledOnValidThread());
}

SkBitmap ClipboardDummyImpl::ReadImage(ui::ClipboardType type) const {
  DCHECK(CalledOnValidThread());
  return SkBitmap();
}

void ClipboardDummyImpl::ReadCustomData(ui::ClipboardType clipboard_type,
                                        const base::string16& type,
                                        base::string16* result) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::ReadBookmark(base::string16* title,
                                      std::string* url) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::ReadData(const FormatType& format,
                                  std::string* result) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::WriteObjects(ui::ClipboardType type,
                                      const ObjectMap& objects) {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::WriteText(const char* text_data, size_t text_len) {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::WriteHTML(const char* markup_data,
                                   size_t markup_len,
                                   const char* url_data,
                                   size_t url_len) {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::WriteRTF(const char* rtf_data, size_t data_len) {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::WriteBookmark(const char* title_data,
                                       size_t title_len,
                                       const char* url_data,
                                       size_t url_len) {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::WriteWebSmartPaste() {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::WriteBitmap(const SkBitmap& bitmap) {
  DCHECK(CalledOnValidThread());
}

void ClipboardDummyImpl::WriteData(const FormatType& format,
                                   const char* data_data,
                                   size_t data_len) {
  DCHECK(CalledOnValidThread());
}

ClipboardDummyImpl::ClipboardDummyImpl() {}

ClipboardDummyImpl::~ClipboardDummyImpl() {}

} // namespace oxide
