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

#include "oxide_qt_clipboard.h"

#include <list>
#include <set>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/ref_counted_memory.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/singleton.h"
#include "base/metrics/histogram.h"
#include "base/stl_util.h"
#include "base/strings/utf_string_conversions.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "ui/base/clipboard/custom_data_helper.h"
#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/events/platform/platform_event_observer.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/gfx/geometry/size.h"

#include <QDebug>
#include <QClipboard>
#include <QGuiApplication>
#include <QImage>
#include <QMimeData>
#include <QObject>
#include <QString>


#define GET_CLIPBOARD_DATA(c) \
  c->mimeData( \
       type == ui::CLIPBOARD_TYPE_COPY_PASTE ? \
       QClipboard::Clipboard \
       : QClipboard::Selection)

class ClipboardChangedListener : public QObject {
  Q_OBJECT
  
 public:
  ClipboardChangedListener();
  uint64_t clipboard_sequence_number() const {
    return clipboard_sequence_number_;
  }
  uint64_t selection_sequence_number() const {
    return selection_sequence_number_;
  }
 private Q_SLOTS:
  void OnClipboardDataChanged(QClipboard::Mode mode);
 private:
  uint64_t clipboard_sequence_number_;
  uint64_t selection_sequence_number_;

  Q_DISABLE_COPY(ClipboardChangedListener);
};

ClipboardChangedListener::ClipboardChangedListener()
  : clipboard_sequence_number_(0),
    selection_sequence_number_(0) {
  QObject::connect(
      QGuiApplication::clipboard(),
      SIGNAL(changed(QClipboard::Mode)),
      this,
      SLOT(OnClipboardDataChanged(QClipboard::Mode)));
}

void ClipboardChangedListener::OnClipboardDataChanged(
      QClipboard::Mode mode) {
  switch (mode) {
    case QClipboard::Clipboard:
      ++clipboard_sequence_number_;
      break;
    case QClipboard::Selection:
      ++selection_sequence_number_;
      break;
    default:
      break;
  }
}

namespace oxide {

namespace qt {

namespace {

const char kMimeTypeWebkitSmartPaste[] = "chromium/x-webkit-paste";

}

///////////////////////////////////////////////////////////////////////////////
// ClipboardAuraX11

uint64_t Clipboard::GetSequenceNumber(ui::ClipboardType type) const {
  DCHECK(CalledOnValidThread());
  return type == ui::CLIPBOARD_TYPE_COPY_PASTE
    ? clipboard_changed_listener_->clipboard_sequence_number()
    : clipboard_changed_listener_->selection_sequence_number();
}

bool Clipboard::IsFormatAvailable(const FormatType& format,
                                  ui::ClipboardType type) const {
  DCHECK(CalledOnValidThread());
  DCHECK(IsSupportedClipboardType(type));

  const QMimeData *data = GET_CLIPBOARD_DATA(QGuiApplication::clipboard());
  if ( ! data) {
    return false;
  }

  return data->hasFormat(format.ToString().c_str());
}

void Clipboard::Clear(ui::ClipboardType type) {
  DCHECK(CalledOnValidThread());
  DCHECK(IsSupportedClipboardType(type));
  QGuiApplication::clipboard()->clear();
}

void Clipboard::ReadAvailableTypes(ui::ClipboardType type,
                                   std::vector<base::string16>* types,
                                   bool* contains_filenames) const {
  DCHECK(IsSupportedClipboardType(type));
  DCHECK(CalledOnValidThread());
  DCHECK(types != nullptr);
  DCHECK(contains_filenames != nullptr);

  const QMimeData *data = GET_CLIPBOARD_DATA(QGuiApplication::clipboard());
  if ( ! data) {
    return;
  }

  *contains_filenames = false;
  Q_FOREACH (const QString& format, data->formats()) {
    types->push_back(base::UTF8ToUTF16(format.toUtf8().data()));

    if (format == GetFilenameFormatType().ToString().c_str()) {
      *contains_filenames = true;
    }
  }
}

void Clipboard::ReadText(ui::ClipboardType type,
                         base::string16* result) const {
  DCHECK(IsSupportedClipboardType(type));
  DCHECK(CalledOnValidThread());

  const QMimeData *data = GET_CLIPBOARD_DATA(QGuiApplication::clipboard());
  if ( ! data || ! data->hasText()) {
    return;
  }

  *result = base::UTF8ToUTF16(data->text().toUtf8().data());
}

void Clipboard::ReadAsciiText(ui::ClipboardType type,
                              std::string* result) const {
  DCHECK(IsSupportedClipboardType(type));
  DCHECK(CalledOnValidThread());

  const QMimeData *data = GET_CLIPBOARD_DATA(QGuiApplication::clipboard());
  if ( ! data || ! data->hasText()) {
    return;
  }

  *result = data->text().toStdString().c_str();
}

void Clipboard::ReadHTML(ui::ClipboardType type,
                         base::string16* markup,
                         std::string* src_url,
                         uint32_t* fragment_start,
                         uint32_t* fragment_end) const {
  DCHECK(IsSupportedClipboardType(type));
  DCHECK(CalledOnValidThread());

  const QMimeData *data = GET_CLIPBOARD_DATA(QGuiApplication::clipboard());
  if ( ! data || ! data->hasHtml()) {
    return;
  }

  *markup = base::UTF8ToUTF16(data->html().toStdString());
  *fragment_start = 0;
  DCHECK(markup->length() <= std::numeric_limits<uint32_t>::max());
  *fragment_end = static_cast<uint32_t>(markup->length());
}

void Clipboard::ReadRTF(ui::ClipboardType type, std::string* result) const {
  DCHECK(IsSupportedClipboardType(type));
  DCHECK(CalledOnValidThread());

  const QMimeData *data = GET_CLIPBOARD_DATA(QGuiApplication::clipboard());
  if ( ! data || ! data->hasFormat(QString::fromLatin1(kMimeTypeRTF))) {
    return;
  }

  *result = data->data(QString::fromLatin1(kMimeTypeRTF)).data();
}

SkBitmap Clipboard::ReadImage(ui::ClipboardType type) const {
  DCHECK(IsSupportedClipboardType(type));
  DCHECK(CalledOnValidThread());

  const QMimeData *md = GET_CLIPBOARD_DATA(QGuiApplication::clipboard());
  if ( ! md) {
    return SkBitmap();
  }

  QImage image;
  if (md->hasImage()) {
    image = qvariant_cast<QImage>(md->imageData());
    /**
     * ReadImage() is called from Blink when 'clipboardData.getAsFile'
     * is called with a forced explicit mime type of 'image/png'.
     * 
     * See third_party/WebKit/Source/core/clipboard/DataObjectItem.cpp
     */
  } else if (md->hasFormat(Clipboard::kMimeTypePNG)) {
    image.loadFromData(md->data(Clipboard::kMimeTypePNG), "PNG");
  } else {
    return SkBitmap();
  }

  if (image.format() != QImage::Format_RGBA8888) {
    image.convertToFormat(QImage::Format_RGBA8888);
  }

  SkBitmap bitmap;
  bitmap.setInfo(
      SkImageInfo::MakeN32(
          image.width(),
          image.height(),
          kOpaque_SkAlphaType));
  
  bitmap.setPixels(const_cast<uchar*>(image.constBits()));

  // Force a deep copy of the image data
  SkBitmap copy;
  bitmap.copyTo(&copy, kN32_SkColorType);
  return copy;
}

void Clipboard::ReadCustomData(ui::ClipboardType type,
                               const base::string16& data_type,
                               base::string16* result) const {
  DCHECK(CalledOnValidThread());
  DCHECK(IsSupportedClipboardType(type));
  
  const QMimeData *data = GET_CLIPBOARD_DATA(QGuiApplication::clipboard());
  QString mime_type = QString::fromStdString(base::UTF16ToUTF8(data_type));
  if ( ! data || ! data->hasFormat(mime_type)) {
    return;
  }

  ui::ReadCustomDataForType(
      data->data(mime_type).constData(),
      data->data(mime_type).size(),
      data_type,
      result);
}

void Clipboard::ReadBookmark(base::string16* title,
                             std::string* url) const {
  NOTIMPLEMENTED();
}

void Clipboard::ReadData(const FormatType& format,
                         std::string* result) const {
  DCHECK(CalledOnValidThread());

  const QMimeData *data = QGuiApplication::clipboard()->mimeData(QClipboard::Clipboard);
  if ( ! data) {
    return;
  }

  *result = data->data(format.ToString().c_str()).data();
}

void Clipboard::WriteObjects(ui::ClipboardType type,
                             const ObjectMap& objects) {
  DCHECK(CalledOnValidThread());
  DCHECK(IsSupportedClipboardType(type));

  if (! write_mime_data_acc_) {
    write_mime_data_acc_.reset(new QMimeData());
  }

  // dispatch all objects and gather the resulting mime data
  for (ObjectMap::const_iterator iter = objects.begin();
       iter != objects.end();
       ++iter) {
    DispatchObject(static_cast<ObjectType>(iter->first), iter->second);
  }

  QGuiApplication::clipboard()->setMimeData(
      write_mime_data_acc_.release(),
      type == ui::CLIPBOARD_TYPE_COPY_PASTE
      ? QClipboard::Clipboard
      : QClipboard::Selection);
}

void Clipboard::WriteText(const char* text_data, size_t text_len) {
  write_mime_data_acc_->setText(QString::fromUtf8(text_data, text_len));
}

void Clipboard::WriteHTML(const char* markup_data,
                          size_t markup_len,
                          const char* url_data,
                          size_t url_len) {
  write_mime_data_acc_->setHtml(
      QString::fromUtf8(markup_data, markup_len));
}

void Clipboard::WriteRTF(const char* rtf_data, size_t data_len) {
  write_mime_data_acc_->setData(QString::fromLatin1(kMimeTypeRTF), QByteArray(rtf_data, data_len));
}

void Clipboard::WriteBookmark(const char* title_data,
                              size_t title_len,
                              const char* url_data,
                              size_t url_len) {
  NOTIMPLEMENTED();
}

void Clipboard::WriteWebSmartPaste() {
  write_mime_data_acc_->setData(
      QString::fromLatin1(kMimeTypeWebkitSmartPaste),
      QByteArray());
}

void Clipboard::WriteBitmap(const SkBitmap& bitmap) {
  QImage image;
  if (bitmap.info().colorType() != kN32_SkColorType) {
    SkImageInfo info =
      SkImageInfo::MakeN32(
        bitmap.width(),
        bitmap.height(),
        bitmap.alphaType());

    SkBitmap convertedBitmap;
    if (!convertedBitmap.tryAllocPixels(info)) {
      return;
    }

    bitmap.readPixels(
        info,
        convertedBitmap.getPixels(),
        0, 0, 0);

    image = QImage(reinterpret_cast<const uchar *>(convertedBitmap.getPixels()),
        bitmap.width(),
        bitmap.height(),
        QImage::Format_RGBA8888);
  } else {
    image = QImage(reinterpret_cast<const uchar *>(bitmap.getPixels()),
        bitmap.width(),
        bitmap.height(),
        QImage::Format_RGBA8888);
  }

  write_mime_data_acc_->setImageData(image.copy());
}

void Clipboard::WriteData(const FormatType& format,
                          const char* data_data,
                          size_t data_len) {
  write_mime_data_acc_->setData(
      QString::fromStdString(format.ToString()),
      QByteArray(data_data, data_len));
}

Clipboard::Clipboard()
  : clipboard_changed_listener_(new ClipboardChangedListener()),
    write_mime_data_acc_(new QMimeData()) {
  DCHECK(CalledOnValidThread());
}

Clipboard::~Clipboard() {
  DCHECK(CalledOnValidThread());
}

} // namespace qt
} // namespace oxide

#include "oxide_qt_clipboard.moc"
