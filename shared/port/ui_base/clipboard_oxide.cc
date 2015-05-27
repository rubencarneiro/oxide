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

#include "clipboard_oxide.h"

#include "base/basictypes.h"
#include "ui/base/clipboard/custom_data_helper.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace ui {

namespace {

const char kMimeTypeFilename[] = "chromium/filename";
const char kMimeTypeBitmap[] = "image/bmp";
const char kMimeTypePepperCustomData[] = "chromium/x-pepper-custom-data";
const char kMimeTypeWebkitSmartPaste[] = "chromium/x-webkit-paste";

ClipboardOxideFactory g_clipboard_factory;

}  // namespace


void SetClipboardOxideFactory(ClipboardOxideFactory factory) {
  g_clipboard_factory = factory;
}

// Clipboard factory method.
Clipboard* Clipboard::Create() {
  if (g_clipboard_factory) {
    return g_clipboard_factory();
  }
  return new ClipboardOxide;
}

  
// Clipboard::FormatType implementation.
Clipboard::FormatType::FormatType() {
}

Clipboard::FormatType::FormatType(const std::string& native_format)
    : data_(native_format) {
}

Clipboard::FormatType::~FormatType() {
}

std::string Clipboard::FormatType::Serialize() const {
  return data_;
}

// static
Clipboard::FormatType Clipboard::FormatType::Deserialize(
    const std::string& serialization) {
  return FormatType(serialization);
}

bool Clipboard::FormatType::operator<(const FormatType& other) const {
  return data_ < other.data_;
}

bool Clipboard::FormatType::Equals(const FormatType& other) const {
  return data_ == other.data_;
}

// Various predefined FormatTypes.
// static
Clipboard::FormatType Clipboard::GetFormatType(
    const std::string& format_string) {
  return FormatType::Deserialize(format_string);
}

// static
const Clipboard::FormatType& Clipboard::GetUrlFormatType() {
  CR_DEFINE_STATIC_LOCAL(FormatType, type, (kMimeTypeURIList));
  return type;
}

// static
const Clipboard::FormatType& Clipboard::GetUrlWFormatType() {
  return GetUrlFormatType();
}

// static
const Clipboard::FormatType& Clipboard::GetPlainTextFormatType() {
  CR_DEFINE_STATIC_LOCAL(FormatType, type, (kMimeTypeText));
  return type;
}

// static
const Clipboard::FormatType& Clipboard::GetPlainTextWFormatType() {
  return GetPlainTextFormatType();
}

// static
const Clipboard::FormatType& Clipboard::GetFilenameFormatType() {
  CR_DEFINE_STATIC_LOCAL(FormatType, type, (kMimeTypeFilename));
  return type;
}

// static
const Clipboard::FormatType& Clipboard::GetFilenameWFormatType() {
  return Clipboard::GetFilenameFormatType();
}

// static
const Clipboard::FormatType& Clipboard::GetHtmlFormatType() {
  CR_DEFINE_STATIC_LOCAL(FormatType, type, (kMimeTypeHTML));
  return type;
}

// static
const Clipboard::FormatType& Clipboard::GetRtfFormatType() {
  CR_DEFINE_STATIC_LOCAL(FormatType, type, (kMimeTypeRTF));
  return type;
}

// static
const Clipboard::FormatType& Clipboard::GetBitmapFormatType() {
  CR_DEFINE_STATIC_LOCAL(FormatType, type, (kMimeTypeBitmap));
  return type;
}

// static
const Clipboard::FormatType& Clipboard::GetWebKitSmartPasteFormatType() {
  CR_DEFINE_STATIC_LOCAL(FormatType, type, (kMimeTypeWebkitSmartPaste));
  return type;
}

// static
const Clipboard::FormatType& Clipboard::GetWebCustomDataFormatType() {
  CR_DEFINE_STATIC_LOCAL(FormatType, type, (kMimeTypeWebCustomData));
  return type;
}

// static
const Clipboard::FormatType& Clipboard::GetPepperCustomDataFormatType() {
  CR_DEFINE_STATIC_LOCAL(FormatType, type, (kMimeTypePepperCustomData));
  return type;
}

ClipboardOxide::ClipboardOxide() {
  DCHECK(CalledOnValidThread());
}

ClipboardOxide::~ClipboardOxide() {
  DCHECK(CalledOnValidThread());
}

uint64 ClipboardOxide::GetSequenceNumber(ClipboardType type) const {
  DCHECK(CalledOnValidThread());
  return uint64();
}

bool ClipboardOxide::IsFormatAvailable(const FormatType& format,
                                      ClipboardType type) const {
  DCHECK(CalledOnValidThread());
  return false;
}

void ClipboardOxide::Clear(ClipboardType type) {
  DCHECK(CalledOnValidThread());
}

void ClipboardOxide::ReadAvailableTypes(ClipboardType type,
                                       std::vector<base::string16>* types,
                                       bool* contains_filenames) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardOxide::ReadText(ClipboardType type, base::string16* result) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardOxide::ReadAsciiText(ClipboardType type,
                                  std::string* result) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardOxide::ReadHTML(ClipboardType type,
                             base::string16* markup,
                             std::string* src_url,
                             uint32* fragment_start,
                             uint32* fragment_end) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardOxide::ReadRTF(ClipboardType type, std::string* result) const {
  DCHECK(CalledOnValidThread());
}

SkBitmap ClipboardOxide::ReadImage(ClipboardType type) const {
  DCHECK(CalledOnValidThread());
  return SkBitmap();
}

void ClipboardOxide::ReadCustomData(ClipboardType clipboard_type,
                                   const base::string16& type,
                                   base::string16* result) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardOxide::ReadBookmark(base::string16* title,
                                 std::string* url) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardOxide::ReadData(const FormatType& format,
                             std::string* result) const {
  DCHECK(CalledOnValidThread());
}

void ClipboardOxide::WriteObjects(ClipboardType type, const ObjectMap& objects) {
  DCHECK(CalledOnValidThread());
}

void ClipboardOxide::WriteText(const char* text_data, size_t text_len) {
}

void ClipboardOxide::WriteHTML(const char* markup_data,
                              size_t markup_len,
                              const char* url_data,
                              size_t url_len) {
}

void ClipboardOxide::WriteRTF(const char* rtf_data, size_t data_len) {
}

void ClipboardOxide::WriteBookmark(const char* title_data,
                                  size_t title_len,
                                  const char* url_data,
                                  size_t url_len) {
}

void ClipboardOxide::WriteWebSmartPaste() {
}

void ClipboardOxide::WriteBitmap(const SkBitmap& bitmap) {
}

void ClipboardOxide::WriteData(const FormatType& format,
                              const char* data_data,
                              size_t data_len) {
}

}  // namespace ui
