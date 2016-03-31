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

#ifndef _QT_TESTS_QMLTESTS_QUICK_TEST_COMPAT_H_
#define _QT_TESTS_QMLTESTS_QUICK_TEST_COMPAT_H_

#include <QObject>

class QTestRootObject : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool windowShown READ windowShown NOTIFY windowShownChanged)
  Q_PROPERTY(bool hasTestCase READ hasTestCase WRITE setHasTestCase NOTIFY hasTestCaseChanged)

 public:
  QTestRootObject(QObject* parent = 0);

  static QTestRootObject* instance();

  bool hasTestCase() const;
  void setHasTestCase(bool value);

  bool windowShown() const;
  void setWindowShown(bool value);

  void reset();

  bool hasQuit() const;

 Q_SIGNALS:
  void windowShownChanged();
  void hasTestCaseChanged();

 private Q_SLOTS:
  void quit();

 private:
  bool has_quit_;
  bool window_shown_;
  bool has_test_case_;
};

#endif // _QT_TESTS_QMLTESTS_QUICK_TEST_COMPAT_H_
