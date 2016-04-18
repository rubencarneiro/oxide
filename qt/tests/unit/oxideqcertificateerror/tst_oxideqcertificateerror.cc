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

struct Params {
  bool isMainFrame = true;
  bool isSubresource = false;
  OxideQCertificateError::Error certError =
      OxideQCertificateError::ErrorBadIdentity;
  bool strictEnforcement = false;
  bool overridable = true;
};

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
  void deleteWithoutResponse();

 private:
  void onResponse(bool response);
  std::unique_ptr<OxideQCertificateError> createError(
      const Params& params = Params());

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
tst_oxideqcertificateerror::createError(const Params& params) {
  return OxideQCertificateErrorPrivate::CreateForTesting(
      params.isMainFrame,
      params.isSubresource,
      params.certError,
      cert_,
      QUrl("https://www.google.com/"),
      params.strictEnforcement,
      params.overridable,
      std::bind(&tst_oxideqcertificateerror::onResponse,
                this, std::placeholders::_1));
}

void tst_oxideqcertificateerror::url() {
  std::unique_ptr<OxideQCertificateError> error = createError();
  QCOMPARE(error->url(), QUrl("https://www.google.com/"));
}

void tst_oxideqcertificateerror::isCancelled() {
  CancelledSpy spy;

  std::unique_ptr<OxideQCertificateError> error = createError();
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
  std::unique_ptr<OxideQCertificateError> error = createError();
  QVERIFY(error->isMainFrame());

  Params params;
  params.isMainFrame = false;

  error = createError(params);
  QVERIFY(!error->isMainFrame());
}

void tst_oxideqcertificateerror::isSubresource() {
  std::unique_ptr<OxideQCertificateError> error = createError();
  QVERIFY(!error->isSubresource());

  Params params;
  params.isSubresource = true;

  error = createError(params);
  QVERIFY(error->isSubresource());
}

void tst_oxideqcertificateerror::overridable() {
  std::unique_ptr<OxideQCertificateError> error = createError();
  QVERIFY(error->overridable());

  error.reset();
  responded_count_ = 0;

  Params params;
  params.overridable = false;

  error = createError(params);
  QVERIFY(!error->overridable());

  error->allow();
  QCOMPARE(responded_count_, 0);

  error.reset();
  QCOMPARE(responded_count_, 0);
}

void tst_oxideqcertificateerror::strictEnforcement() {
  std::unique_ptr<OxideQCertificateError> error = createError();
  QVERIFY(!error->strictEnforcement());

  Params params;
  params.strictEnforcement = true;

  error = createError(params);
  QVERIFY(error->strictEnforcement());
}

void tst_oxideqcertificateerror::certificate() {
  std::unique_ptr<OxideQCertificateError> error = createError();
  QCOMPARE(error->certificate().subjectDisplayName(), cert_.subjectDisplayName());
  QCOMPARE(error->certificate().issuerDisplayName(), cert_.issuerDisplayName());
}

void tst_oxideqcertificateerror::certError() {
  std::unique_ptr<OxideQCertificateError> error = createError();
  QCOMPARE(error->certError(), OxideQCertificateError::ErrorBadIdentity);

  Params params;
  params.certError = OxideQCertificateError::ErrorRevoked;

  error = createError(params);
  QCOMPARE(error->certError(), OxideQCertificateError::ErrorRevoked);
}

void tst_oxideqcertificateerror::allow() {
  std::unique_ptr<OxideQCertificateError> error = createError();

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
  std::unique_ptr<OxideQCertificateError> error = createError();

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

void tst_oxideqcertificateerror::deleteWithoutResponse() {
  std::unique_ptr<OxideQCertificateError> error = createError();
  error.reset();

  QCOMPARE(responded_count_, 1);
  QVERIFY(!last_response_);
}

QTEST_MAIN(tst_oxideqcertificateerror)

#include "tst_oxideqcertificateerror.moc"
