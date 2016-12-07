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

#include <QLatin1String>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QtQml/private/qqmlvaluetype_p.h> // for qmlRegisterValueTypeEnums

#include "qt/core/api/oxideqwebcontextmenuparams.h"
#include "qt/quick/api/oxideqquickwebview.h"
#include "qt/uitk/lib/api/oxideubuntuwebcontextmenu.h"
#include "qt/uitk/lib/api/oxideubuntuwebcontextmenuitem.h"
#include "qt/uitk/lib/api/oxideubuntuwebview.h"

class OxideUbuntuQmlPlugin : public QQmlExtensionPlugin {
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface" FILE "oxide_ubuntu_qml_plugin.json")
  Q_OBJECT

 public:
  void registerTypes(const char* uri) {
    Q_ASSERT(QLatin1String(uri) == QLatin1String("Oxide.Ubuntu"));

    qWarning() <<
        "Oxide.Ubuntu is an experimental module - future versions may "
        "change in backwards-incompatible ways, and without bumping the module "
        "version. Please use with caution";

    qRegisterMetaType<OxideQWebContextMenuParams>();
    qmlRegisterValueTypeEnums<OxideQWebContextMenuParams>(
        uri, 1, 0, "WebContextMenuParams");

    qmlRegisterType<OxideUbuntuWebContextMenu>(
        uri, 1, 0, "UbuntuWebContextMenu");
    qmlRegisterType<OxideUbuntuWebContextMenuItem>(
        uri, 1, 0, "UbuntuWebContextMenuItem");
    qmlRegisterType<OxideUbuntuWebView>(uri, 1, 0, "UbuntuWebView");
    qmlRegisterRevision<OxideQQuickWebView, 9>(uri, 1, 0);
  }
};

#include "oxide_ubuntu_qml_plugin.moc"
