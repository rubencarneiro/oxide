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

#ifndef OXIDE_QTCORE_WEB_CONTEXT_MENU_PARAMS
#define OXIDE_QTCORE_WEB_CONTEXT_MENU_PARAMS

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>
#include <QtCore/QtGlobal>
#include <QtCore/QUrl>

#include <OxideQtCore/oxideqglobal.h>

class OxideQWebContextMenuParamsData;

class OXIDE_QTCORE_EXPORT OxideQWebContextMenuParams {
  Q_GADGET
  Q_PROPERTY(QUrl pageUrl READ pageUrl)
  Q_PROPERTY(QUrl frameUrl READ frameUrl)

  Q_PROPERTY(bool isLink READ isLink)
  Q_PROPERTY(bool isEditable READ isEditable)
  Q_PROPERTY(bool isSelection READ isSelection)
  Q_PROPERTY(MediaType mediaType READ mediaType)

  Q_PROPERTY(QUrl linkUrl READ linkUrl)
  Q_PROPERTY(QString linkText READ linkText)
  Q_PROPERTY(QString titleText READ titleText)
  Q_PROPERTY(QUrl srcUrl READ srcUrl)

  Q_ENUMS(MediaType)

 public:
  enum MediaType {
    MediaTypeNone,
    MediaTypeImage,
    MediaTypeVideo,
    MediaTypeAudio,
    MediaTypeCanvas,
    MediaTypePlugin
  };

  OxideQWebContextMenuParams();
  OxideQWebContextMenuParams(const OxideQWebContextMenuParams& other);
  ~OxideQWebContextMenuParams();

  OxideQWebContextMenuParams& operator=(const OxideQWebContextMenuParams& other);
  bool operator==(const OxideQWebContextMenuParams& other) const;
  bool operator!=(const OxideQWebContextMenuParams& other) const;

  QUrl pageUrl() const;
  QUrl frameUrl() const;

  bool isLink() const;
  bool isEditable() const;
  bool isSelection() const;
  MediaType mediaType() const;

  QUrl linkUrl() const;
  QString linkText() const;
  QString titleText() const;
  QUrl srcUrl() const;

 private:
  friend class OxideQWebContextMenuParamsData;
  OxideQWebContextMenuParams(
      QSharedDataPointer<OxideQWebContextMenuParamsData> d);

  QSharedDataPointer<OxideQWebContextMenuParamsData> d;
};

Q_DECLARE_METATYPE(OxideQWebContextMenuParams);

#endif // OXIDE_QTCORE_WEB_CONTEXT_MENU_PARAMS
