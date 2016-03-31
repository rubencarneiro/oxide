// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "quick_test_compat.h"

#include <QPointer>
#include <QtDebug>

void QTestRootObject::quit() {
  has_quit_ = true;
}

QTestRootObject::QTestRootObject(QObject* parent)
    : QObject(parent),
      has_quit_(false),
      window_shown_(false),
      has_test_case_(false) {}

// static
QTestRootObject* QTestRootObject::instance() {
  static QPointer<QTestRootObject> object = new QTestRootObject();
  Q_ASSERT(object);
  return object;
}

bool QTestRootObject::hasTestCase() const {
  return has_test_case_;
}

void QTestRootObject::setHasTestCase(bool value) {
  has_test_case_ = value;
  Q_EMIT hasTestCaseChanged();
}

bool QTestRootObject::windowShown() const {
  return window_shown_;
}

void QTestRootObject::setWindowShown(bool value) {
  window_shown_ = value;
  Q_EMIT windowShownChanged();
}

void QTestRootObject::reset() {
  setWindowShown(false);
  setHasTestCase(false);
  has_quit_ = false;
}

bool QTestRootObject::hasQuit() const {
  return has_quit_;
}
