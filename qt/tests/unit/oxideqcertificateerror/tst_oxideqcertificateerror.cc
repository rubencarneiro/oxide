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

#include <functional>
#include <memory>
#include <QDateTime>
#include <QTest>
#include <QUrl>

#include "qt/core/api/oxideqcertificateerror.h"
#include "qt/core/api/oxideqcertificateerror_p.h"
#include "qt/core/api/oxideqsslcertificate.h"
#include "qt/core/api/oxideqsslcertificate_p.h"

class CancelledSpy : public QObject {
  Q_OBJECT

 public:
  CancelledSpy() : count_(0) {}

  int count() const { return count_; }

 private Q_SLOTS:
  void onCancelled();

 private:
  int count_;
};

void CancelledSpy::onCancelled() {
  ++count_;
}

class tst_oxideqcertificateerror : public QObject {
  Q_OBJECT

 public:
  tst_oxideqcertificateerror()
      : responded_count_(0),
        last_response_(false) {}

 private Q_SLOTS:
  void initTestCase();
  void init();

  void url();
  void isCancelled();
  void isMainFrame();
  void isSubresource();
  void overridable();
  void strictEnforcement();
  void certificate();
  void certError();
  void allow();
  void deny();

 private:
  void onResponse(bool response);
  std::unique_ptr<OxideQCertificateError> makeOverridable();

  OxideQSslCertificate cert_;

  int responded_count_;
  bool last_response_;
};

void tst_oxideqcertificateerror::initTestCase() {
  cert_ =
      OxideQSslCertificateData::CreateForTesting("https://www.google.com/",
                                                 "https://www.example.com/",
                                                 QDateTime(), QDateTime());
}

void tst_oxideqcertificateerror::init() {
  responded_count_ = 0;
  last_response_ = false;
}

void tst_oxideqcertificateerror::onResponse(bool response) {
  ++responded_count_;
  last_response_ = response;
}

std::unique_ptr<OxideQCertificateError>
tst_oxideqcertificateerror::makeOverridable() {
  return OxideQCertificateErrorPrivate::CreateForTesting(
      true,
      false,
      OxideQCertificateError::ErrorBadIdentity,
      cert_,
      QUrl("https://www.google.com/"),
      false,
      true,
      std::bind(&tst_oxideqcertificateerror::onResponse,
                this, std::placeholders::_1));
}

void tst_oxideqcertificateerror::url() {
  std::unique_ptr<OxideQCertificateError> error = makeOverridable();
  QCOMPARE(error->url(), QUrl("https://www.google.com/"));
}

void tst_oxideqcertificateerror::isCancelled() {
  CancelledSpy spy;

  std::unique_ptr<OxideQCertificateError> error = makeOverridable();
  spy.connect(error.get(), SIGNAL(cancelled()), SLOT(onCancelled()));

  QVERIFY(!error->isCancelled());

  OxideQCertificateErrorPrivate::get(error.get())->SimulateCancel();
  QVERIFY(error->isCancelled());
  QCOMPARE(spy.count(), 1);
  QCOMPARE(responded_count_, 1);

  error->allow();
  QCOMPARE(responded_count_, 1);

  error.reset();
  QCOMPARE(responded_count_, 1);
}

void tst_oxideqcertificateerror::isMainFrame() {
  std::unique_ptr<OxideQCertificateError> error = makeOverridable();
  QVERIFY(error->isMainFrame());

  error = OxideQCertificateErrorPrivate::CreateForTesting(
      false,
      false,
      OxideQCertificateError::ErrorBadIdentity,
      cert_,
      QUrl("https://www.google.com/"),
      false,
      true,
      std::bind(&tst_oxideqcertificateerror::onResponse,
                this, std::placeholders::_1));
  QVERIFY(!error->isMainFrame());
}

void tst_oxideqcertificateerror::isSubresource() {
  std::unique_ptr<OxideQCertificateError> error = makeOverridable();
  QVERIFY(!error->isSubresource());

  error = OxideQCertificateErrorPrivate::CreateForTesting(
      true,
      true,
      OxideQCertificateError::ErrorBadIdentity,
      cert_,
      QUrl("https://www.google.com/"),
      false,
      true,
      std::bind(&tst_oxideqcertificateerror::onResponse,
                this, std::placeholders::_1));
  QVERIFY(error->isSubresource());
}

void tst_oxideqcertificateerror::overridable() {
  std::unique_ptr<OxideQCertificateError> error = makeOverridable();
  QVERIFY(error->overridable());

  error.reset();
  responded_count_ = 0;

  error = OxideQCertificateErrorPrivate::CreateForTesting(
      true,
      false,
      OxideQCertificateError::ErrorBadIdentity,
      cert_,
      QUrl("https://www.google.com/"),
      false,
      false,
      std::bind(&tst_oxideqcertificateerror::onResponse,
                this, std::placeholders::_1));
  QVERIFY(!error->overridable());

  error->allow();
  QCOMPARE(responded_count_, 0);

  error.reset();
  QCOMPARE(responded_count_, 0);
}

void tst_oxideqcertificateerror::strictEnforcement() {
  std::unique_ptr<OxideQCertificateError> error = makeOverridable();
  QVERIFY(!error->strictEnforcement());

  error = OxideQCertificateErrorPrivate::CreateForTesting(
      true,
      false,
      OxideQCertificateError::ErrorBadIdentity,
      cert_,
      QUrl("https://www.google.com/"),
      true,
      true,
      std::bind(&tst_oxideqcertificateerror::onResponse,
                this, std::placeholders::_1));
  QVERIFY(error->strictEnforcement());
}

void tst_oxideqcertificateerror::certificate() {
  std::unique_ptr<OxideQCertificateError> error = makeOverridable();
  QCOMPARE(error->certificate().subjectDisplayName(), cert_.subjectDisplayName());
  QCOMPARE(error->certificate().issuerDisplayName(), cert_.issuerDisplayName());
}

void tst_oxideqcertificateerror::certError() {
  std::unique_ptr<OxideQCertificateError> error = makeOverridable();
  QCOMPARE(error->certError(), OxideQCertificateError::ErrorBadIdentity);

  error = OxideQCertificateErrorPrivate::CreateForTesting(
      true,
      false,
      OxideQCertificateError::ErrorRevoked,
      cert_,
      QUrl("https://www.google.com/"),
      false,
      true,
      std::bind(&tst_oxideqcertificateerror::onResponse,
                this, std::placeholders::_1));
  QCOMPARE(error->certError(), OxideQCertificateError::ErrorRevoked);
}

void tst_oxideqcertificateerror::allow() {
  std::unique_ptr<OxideQCertificateError> error = makeOverridable();

  error->allow();
  QCOMPARE(responded_count_, 1);
  QVERIFY(last_response_);

  error->allow();
  QCOMPARE(responded_count_, 1);

  error->deny();
  QCOMPARE(responded_count_, 1);

  error.reset();
  QCOMPARE(responded_count_, 1);
}

void tst_oxideqcertificateerror::deny() {
  std::unique_ptr<OxideQCertificateError> error = makeOverridable();

  error->deny();
  QCOMPARE(responded_count_, 1);
  QVERIFY(!last_response_);

  error->deny();
  QCOMPARE(responded_count_, 1);

  error->allow();
  QCOMPARE(responded_count_, 1);

  error.reset();
  QCOMPARE(responded_count_, 1);
}

QTEST_MAIN(tst_oxideqcertificateerror)

#include "tst_oxideqcertificateerror.moc"
