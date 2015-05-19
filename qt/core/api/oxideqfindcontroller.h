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

#ifndef OXIDE_Q_FIND_CONTROLLER
#define OXIDE_Q_FIND_CONTROLLER

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

class OxideQFindControllerPrivate;

namespace oxide {
  class WebView;
  namespace qt {
      class WebView;
  }
}

class Q_DECL_EXPORT OxideQFindController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(bool caseSensitive READ caseSensitive WRITE setCaseSensitive NOTIFY caseSensitiveChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int current READ current NOTIFY currentChanged)

    Q_DECLARE_PRIVATE(OxideQFindController)
    Q_DISABLE_COPY(OxideQFindController)

   public:
    OxideQFindController(oxide::WebView* webview);
    ~OxideQFindController();

    const QString& text() const;
    void setText(const QString& text);
    bool caseSensitive() const;
    void setCaseSensitive(bool caseSensitive);
    int count() const;
    int current() const;

    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();

   Q_SIGNALS:
    void textChanged() const;
    void caseSensitiveChanged() const;
    void countChanged() const;
    void currentChanged() const;

   protected:
    void updateOnFindResult(int current, int count);
    void updateOnParametersChanged();

   private:
    QScopedPointer<OxideQFindControllerPrivate> d_ptr;

   friend class oxide::qt::WebView;
};

#endif // OXIDE_Q_FIND_CONTROLLER
