// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _OXIDE_QT_CORE_BROWSER_CLIPBOARD_QT_H_
#define _OXIDE_QT_CORE_BROWSER_CLIPBOARD_QT_H_

#include "ui/base/clipboard/clipboard.h"

#include "base/memory/scoped_ptr.h"

class QMimeData;
class ClipboardChangedListener;

namespace oxide {
namespace qt {

class ClipboardQt : public ui::Clipboard {
 public:

  static Clipboard* DoCreate();
  
 private:
  friend class Clipboard;

  ClipboardQt();
  ~ClipboardQt() override;

  // Clipboard overrides

  uint64 GetSequenceNumber(ui::ClipboardType type) const override;
  bool IsFormatAvailable(const FormatType& format,
                         ui::ClipboardType type) const override;
  void Clear(ui::ClipboardType type) override;
  void ReadAvailableTypes(ui::ClipboardType type,
                          std::vector<base::string16>* types,
                          bool* contains_filenames) const override;
  void ReadText(ui::ClipboardType type, base::string16* result) const override;
  void ReadAsciiText(ui::ClipboardType type, std::string* result) const override;
  void ReadHTML(ui::ClipboardType type,
                base::string16* markup,
                std::string* src_url,
                uint32* fragment_start,
                uint32* fragment_end) const override;
  void ReadRTF(ui::ClipboardType type, std::string* result) const override;
  SkBitmap ReadImage(ui::ClipboardType type) const override;
  void ReadCustomData(ui::ClipboardType clipboard_type,
                      const base::string16& type,
                      base::string16* result) const override;
  void ReadBookmark(base::string16* title, std::string* url) const override;
  void ReadData(const FormatType& format, std::string* result) const override;
  void WriteObjects(ui::ClipboardType type, const ui::Clipboard::ObjectMap& objects) override;
  void WriteText(const char* text_data, size_t text_len) override;
  void WriteHTML(const char* markup_data,
                 size_t markup_len,
                 const char* url_data,
                 size_t url_len) override;
  void WriteRTF(const char* rtf_data, size_t data_len) override;
  void WriteBookmark(const char* title_data,
                     size_t title_len,
                     const char* url_data,
                     size_t url_len) override;
  void WriteWebSmartPaste() override;
  void WriteBitmap(const SkBitmap& bitmap) override;
  void WriteData(const FormatType& format,
                 const char* data_data,
                 size_t data_len) override;

 private:

  scoped_ptr<ClipboardChangedListener> clipboard_changed_listener_;
  
  // Used for accumulated mimedata
  scoped_ptr<QMimeData> write_mime_data_acc_;
  
  DISALLOW_COPY_AND_ASSIGN(ClipboardQt);
};

}  // namespace qt
}  // namespace oxide

#endif  // _OXIDE_QT_CORE_BROWSER_CLIPBOARD_QT_H_
