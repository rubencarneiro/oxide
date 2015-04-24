// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_qt_clipboard.h"

#include <list>
#include <set>

#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/ref_counted_memory.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/singleton.h"
#include "base/metrics/histogram.h"
#include "base/stl_util.h"
#include "base/strings/utf_string_conversions.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/base/clipboard/custom_data_helper.h"
#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/events/platform/platform_event_observer.h"
#include "ui/events/platform/platform_event_source.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/gfx/geometry/size.h"

#include <QObject>
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QString>
#include <QImage>

class ClipboardChangedListener : public QObject {
  Q_OBJECT
  
 public:
  ClipboardChangedListener();
  uint64 clipboard_sequence_number() const {
    return clipboard_sequence_number_;
  }
  uint64 selection_sequence_number() const {
    return selection_sequence_number_;
  }
private Q_SLOTS:
  void OnClipboardDataChanged(QClipboard::Mode mode);
private:
  uint64 clipboard_sequence_number_;
  uint64 selection_sequence_number_;

  DISALLOW_COPY_AND_ASSIGN(ClipboardChangedListener);
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

ClipboardQt::ClipboardQt()
  : clipboard_changed_listener_(new ClipboardChangedListener()),
    write_mime_data_acc_(new QMimeData()) {
  DCHECK(CalledOnValidThread());
}

ui::Clipboard* ClipboardQt::DoCreate() {
  return new ClipboardQt();
}

ClipboardQt::~ClipboardQt() {
  DCHECK(CalledOnValidThread());
}

uint64 ClipboardQt::GetSequenceNumber(ui::ClipboardType type) const {
  DCHECK(CalledOnValidThread());
  return type == ui::CLIPBOARD_TYPE_COPY_PASTE
    ? clipboard_changed_listener_->clipboard_sequence_number()
    : clipboard_changed_listener_->selection_sequence_number();
}

bool ClipboardQt::IsFormatAvailable(const FormatType& format,
                                    ui::ClipboardType type) const {
  DCHECK(CalledOnValidThread());
  DCHECK(IsSupportedClipboardType(type));

  QClipboard* clipboard = QGuiApplication::clipboard();
  DCHECK(clipboard);
  if ( ! clipboard) {
    LOG(ERROR) << "Could not access clipboard";
    return false;
  }

  const QMimeData *data =
    clipboard->mimeData(
       type == ui::CLIPBOARD_TYPE_COPY_PASTE ?
       QClipboard::Clipboard
       : QClipboard::Selection);
  if ( ! data) {
    DCHECK(data);
    return false;
  }

  return data->hasFormat(format.ToString().c_str());
}

void ClipboardQt::Clear(ui::ClipboardType type) {
  DCHECK(CalledOnValidThread());
  DCHECK(IsSupportedClipboardType(type));

  QClipboard* clipboard = QGuiApplication::clipboard();
  DCHECK(clipboard);
  if ( ! clipboard) {
    LOG(ERROR) << "Could not access clipboard";
    return;
  }

  clipboard->clear();
}

void ClipboardQt::ReadAvailableTypes(ui::ClipboardType type,
                                     std::vector<base::string16>* types,
                                     bool* contains_filenames) const {
  DCHECK(IsSupportedClipboardType(type));
  DCHECK(CalledOnValidThread());
  if (!types || !contains_filenames) {
    NOTREACHED();
    return;
  }

  QClipboard* clipboard = QGuiApplication::clipboard();
  DCHECK(clipboard);
  if ( ! clipboard) {
    LOG(ERROR) << "Could not access clipboard";
    return;
  }

  const QMimeData *data =
    clipboard->mimeData(
       type == ui::CLIPBOARD_TYPE_COPY_PASTE ?
       QClipboard::Clipboard
       : QClipboard::Selection);
  if ( ! data) {
    return;
  }

  std::vector<base::string16> out_types;
  Q_FOREACH (const QString& format, data->formats()) {
    out_types.push_back(base::UTF8ToUTF16(format.toUtf8().data()));
  }
  std::swap(*types, out_types);

  *contains_filenames = false;
}

void ClipboardQt::ReadText(ui::ClipboardType type,
                           base::string16* result) const {
  DCHECK(IsSupportedClipboardType(type));
  DCHECK(CalledOnValidThread());

  QClipboard* clipboard = QGuiApplication::clipboard();
  DCHECK(clipboard);
  if ( ! clipboard) {
    LOG(ERROR) << "Could not access clipboard";
    return;
  }

  const QMimeData *data =
    clipboard->mimeData(
       type == ui::CLIPBOARD_TYPE_COPY_PASTE ?
       QClipboard::Clipboard
       : QClipboard::Selection);
  if ( ! data || ! data->hasText()) {
    return;
  }

  *result = base::UTF8ToUTF16(data->text().toUtf8().data());
}

  void ClipboardQt::ReadAsciiText(ui::ClipboardType type,
                                std::string* result) const {
  DCHECK(IsSupportedClipboardType(type));
  DCHECK(CalledOnValidThread());

  QClipboard* clipboard = QGuiApplication::clipboard();
  DCHECK(clipboard);
  if ( ! clipboard) {
    LOG(ERROR) << "Could not access clipboard";
    return;
  }

  const QMimeData *data =
    clipboard->mimeData(
       type == ui::CLIPBOARD_TYPE_COPY_PASTE ?
       QClipboard::Clipboard
       : QClipboard::Selection);
  if ( ! data || ! data->hasText()) {
    return;
  }

  *result = data->text().toStdString().c_str();
}

void ClipboardQt::ReadHTML(ui::ClipboardType type,
                           base::string16* markup,
                           std::string* src_url,
                           uint32* fragment_start,
                           uint32* fragment_end) const {
  DCHECK(IsSupportedClipboardType(type));
  DCHECK(CalledOnValidThread());

  QClipboard* clipboard = QGuiApplication::clipboard();
  DCHECK(clipboard);
  if ( ! clipboard) {
    LOG(ERROR) << "Could not access clipboard";
    return;
  }

  const QMimeData *data =
    clipboard->mimeData(
       type == ui::CLIPBOARD_TYPE_COPY_PASTE ?
       QClipboard::Clipboard
       : QClipboard::Selection);
  if ( ! data || ! data->hasHtml()) {
    return;
  }

  markup->clear();
  if (src_url)
    src_url->clear();

  *markup = base::UTF8ToUTF16(data->html().toStdString());
  *fragment_start = 0;
  DCHECK(markup->length() <= kuint32max);
  *fragment_end = static_cast<uint32>(markup->length());
}

void ClipboardQt::ReadRTF(ui::ClipboardType type, std::string* result) const {
  DCHECK(IsSupportedClipboardType(type));
  DCHECK(CalledOnValidThread());

  QClipboard* clipboard = QGuiApplication::clipboard();
  DCHECK(clipboard);
  if ( ! clipboard) {
    LOG(ERROR) << "Could not access clipboard";
    return;
  }

  const QMimeData *data =
    clipboard->mimeData(
       type == ui::CLIPBOARD_TYPE_COPY_PASTE ?
       QClipboard::Clipboard
       : QClipboard::Selection);
  if ( ! data || ! data->hasFormat(QString::fromLatin1(kMimeTypeRTF))) {
    return;
  }

  *result = data->data(QString::fromLatin1(kMimeTypeRTF)).data();
}

SkBitmap ClipboardQt::ReadImage(ui::ClipboardType type) const {
  DCHECK(IsSupportedClipboardType(type));
  DCHECK(CalledOnValidThread());

  QClipboard* clipboard = QGuiApplication::clipboard();
  DCHECK(clipboard);
  if ( ! clipboard) {
    LOG(ERROR) << "Could not access clipboard";
    return SkBitmap();
  }

  const QMimeData *data =
    clipboard->mimeData(
       type == ui::CLIPBOARD_TYPE_COPY_PASTE ?
       QClipboard::Clipboard
       : QClipboard::Selection);
  if ( ! data || ! data->hasImage()) {
    return SkBitmap();
  }

  QImage image = qvariant_cast<QImage>(data->imageData());

  Q_ASSERT(image.format() == QImage::Format_ARGB32);
  SkBitmap bitmap;
  bitmap.setInfo(
      SkImageInfo::MakeN32(
          image.width(),
          image.height(),
          kOpaque_SkAlphaType));

  bitmap.setPixels(const_cast<uchar*>(image.constBits()));

  SkBitmap copy;
  bitmap.copyTo(&copy, kN32_SkColorType);
  return copy;
}

void ClipboardQt::ReadCustomData(ui::ClipboardType clipboard_type,
                                 const base::string16& type,
                                 base::string16* result) const {
  DCHECK(CalledOnValidThread());
  DCHECK(IsSupportedClipboardType(clipboard_type));
  
  QClipboard* clipboard = QGuiApplication::clipboard();
  DCHECK(clipboard);
  if ( ! clipboard) {
    LOG(ERROR) << "Could not access clipboard";
    return;
  }

  const QMimeData *data =
    clipboard->mimeData(
       clipboard_type == ui::CLIPBOARD_TYPE_COPY_PASTE ?
       QClipboard::Clipboard
       : QClipboard::Selection);
  QString mime_type = QString::fromStdString(base::UTF16ToUTF8(type));
  if ( ! data || ! data->hasFormat(mime_type)) {
    return;
  }

  ui::ReadCustomDataForType(
      data->data(mime_type).constData(),
      data->data(mime_type).size(),
      type,
      result);
}

void ClipboardQt::ReadBookmark(base::string16* title,
                               std::string* url) const {
  DCHECK(CalledOnValidThread());
  NOTIMPLEMENTED();
}

void ClipboardQt::ReadData(const FormatType& format,
                           std::string* result) const {
  DCHECK(CalledOnValidThread());

  QClipboard* clipboard = QGuiApplication::clipboard();
  DCHECK(clipboard);
  if ( ! clipboard) {
    LOG(ERROR) << "Could not access clipboard";
    return;
  }

  const QMimeData *data =
    clipboard->mimeData(QClipboard::Clipboard);
  if ( ! data) {
    return;
  }

  *result = data->data(format.ToString().c_str()).data();
}

void ClipboardQt::EnsureAccumulatorMimeDataExists() {
  DCHECK(CalledOnValidThread());

  if (! write_mime_data_acc_) {
    write_mime_data_acc_.reset(new QMimeData());
  }
}

void ClipboardQt::WriteObjects(ui::ClipboardType type,
                               const ObjectMap& objects) {
  DCHECK(CalledOnValidThread());
  DCHECK(IsSupportedClipboardType(type));

  EnsureAccumulatorMimeDataExists();

  // dispatch all objects and gather the resulting mime data

  for (ObjectMap::const_iterator iter = objects.begin();
       iter != objects.end();
       ++iter) {
    DispatchObject(static_cast<ObjectType>(iter->first), iter->second);
  }

  QClipboard* clipboard = QGuiApplication::clipboard();
  DCHECK(clipboard);
  if ( ! clipboard) {
    LOG(ERROR) << "Could not access clipboard";
    return;
  }

  clipboard->setMimeData(
      write_mime_data_acc_.release(),
      type == ui::CLIPBOARD_TYPE_COPY_PASTE
      ? QClipboard::Clipboard
      : QClipboard::Selection);
}

void ClipboardQt::WriteText(const char* text_data, size_t text_len) {
  write_mime_data_acc_->setText(QString::fromUtf8(text_data, text_len));
}

void ClipboardQt::WriteHTML(const char* markup_data,
                                 size_t markup_len,
                                 const char* url_data,
                                 size_t url_len) {
  // TODO : Expand relative links with |url_data|.

  write_mime_data_acc_->setHtml(
      QString::fromUtf8(markup_data, markup_len));
}

void ClipboardQt::WriteRTF(const char* rtf_data, size_t data_len) {
  write_mime_data_acc_->setData(QString::fromLatin1(kMimeTypeRTF), QByteArray(rtf_data, data_len));
}

void ClipboardQt::WriteBookmark(const char* title_data,
                                     size_t title_len,
                                     const char* url_data,
                                     size_t url_len) {
  NOTIMPLEMENTED();
}

void ClipboardQt::WriteWebSmartPaste() {
  write_mime_data_acc_->setData(
      QString::fromLatin1(kMimeTypeWebkitSmartPaste),
      QByteArray());
}

void ClipboardQt::WriteBitmap(const SkBitmap& bitmap) {
  QImage image(reinterpret_cast<const uchar *>(bitmap.getPixels()),
      bitmap.width(),
      bitmap.height(),
      QImage::Format_ARGB32);

  write_mime_data_acc_->setImageData(image.copy());
}

void ClipboardQt::WriteData(const FormatType& format,
                                 const char* data_data,
                                 size_t data_len) {
  write_mime_data_acc_->setData(
      QString::fromStdString(format.ToString()),
      QByteArray(data_data, data_len));
}

} // namespace qt
} // namespace oxide

#include "oxide_qt_clipboard.moc"
