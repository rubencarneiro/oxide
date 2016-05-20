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

#ifndef _OXIDE_QT_CORE_BROWSER_CLIPBOARD_QT_H_
#define _OXIDE_QT_CORE_BROWSER_CLIPBOARD_QT_H_

#include <memory>
#include <QClipboard>
#include <QObject>
#include <QtGlobal>

#include "shared/browser/clipboard/oxide_clipboard.h"

QT_BEGIN_NAMESPACE
class QMimeData;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class Clipboard : public QObject,
                  public oxide::Clipboard {
  Q_OBJECT

 public:
  Clipboard();
  ~Clipboard() override;

 private Q_SLOTS:
  void OnClipboardChanged(QClipboard::Mode mode);
  void OnClipboardDataChanged();

 private:
  // ui::Clipboard overrides
  uint64_t GetSequenceNumber(ui::ClipboardType type) const override;
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
                uint32_t* fragment_start,
                uint32_t* fragment_end) const override;
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

  uint64_t clipboard_sequence_number_;
  uint64_t selection_sequence_number_;

  // Used for accumulated mimedata
  std::unique_ptr<QMimeData> write_mime_data_acc_;
  
  DISALLOW_COPY_AND_ASSIGN(Clipboard);
};

}  // namespace qt
}  // namespace oxide

#endif  // _OXIDE_QT_CORE_BROWSER_CLIPBOARD_QT_H_
