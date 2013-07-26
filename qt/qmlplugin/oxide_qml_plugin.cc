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

#include <QString>
#include <QtGlobal>
#include <QtQml>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "qt/lib/browser/oxide_qquick_web_view.h"
#include "qt/lib/browser/oxide_qquick_web_view_context.h"

QT_USE_NAMESPACE

namespace {

QObject* WebViewContextSingletonFactory(QQmlEngine* engine,
                                        QJSEngine* script_engine) {
  Q_UNUSED(engine);
  Q_UNUSED(script_engine);

  return new OxideQQuickWebViewContext();
}

}

class OxideQmlPlugin : public QQmlExtensionPlugin {
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface" FILE "oxide_qml_plugin.json")
  Q_OBJECT
 public:
  void registerTypes(const char* uri) {
    Q_ASSERT(QLatin1String(uri) == QLatin1String("com.canonical.Oxide"));

    qmlRegisterSingletonType<OxideQQuickWebViewContext>(
        uri, 0, 1, "WebViewContext", WebViewContextSingletonFactory);
    qmlRegisterType<OxideQQuickWebView>(uri, 0, 1, "WebView");
  }
};

#include "oxide_qml_plugin.moc"
