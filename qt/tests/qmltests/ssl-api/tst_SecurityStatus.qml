import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

Item {
  id: top

  SignalSpy {
    id: securityLevelSpy
    signalName: "securityLevelChanged"
  }
  SignalSpy {
    id: contentStatusSpy
    signalName: "contentStatusChanged"
  }
  SignalSpy {
    id: certStatusSpy
    signalName: "certStatusChanged"
  }
  SignalSpy {
    id: certificateSpy
    signalName: "certificateChanged"
  }

  TestWebContext {
    id: context
  }

  Component {
    id: webViewFactory
    TestWebView {
      context: context

      preferences.canRunInsecureContent: true

      onCertificateError: {
        error.allow();
      }
    }
  }

  TestCase {
    id: test
    name: "SecurityStatus"
    when: windowShown

    function init() {
      securityLevelSpy.clear();
      contentStatusSpy.clear();
      certStatusSpy.clear();
      certificateSpy.clear();
    }

    function create_webview() {
      var webView = webViewFactory.createObject(top, {});

      securityLevelSpy.target = webView.securityStatus;
      contentStatusSpy.target = webView.securityStatus;
      certStatusSpy.target = webView.securityStatus;
      certificateSpy.target = webView.securityStatus;

      compare(webView.securityStatus.securityLevel, SecurityStatus.SecurityLevelNone);
      compare(webView.securityStatus.contentStatus, SecurityStatus.ContentStatusNormal);
      compare(webView.securityStatus.certStatus, SecurityStatus.CertStatusOk);
      verify(!webView.securityStatus.certificate);

      return webView;
    }

    function test_SecurityStatus1_main_data() {
      // XXX: How do we test that EV works?
      return [
        {
          url: "http://localhost:8080/empty.html",
          securityLevel: SecurityStatus.SecurityLevelNone,
          contentStatus: SecurityStatus.ContentStatusNormal,
          certStatus: SecurityStatus.CertStatusOk,
          securityLevelSignals: 0, contentStatusSignals: 0,
          certStatusSignals: 0, certificateSignals: 0,
          certificate: false
        },
        {
          url: "https://localhost:4443/empty.html",
          securityLevel: SecurityStatus.SecurityLevelSecure,
          contentStatus: SecurityStatus.ContentStatusNormal,
          certStatus: SecurityStatus.CertStatusOk,
          securityLevelSignals: 1, contentStatusSignals: 0,
          certStatusSignals: 0, certificateSignals: 1,
          certificate: true
        },
        {
          url: "https://localhost:4444/empty.html",
          securityLevel: SecurityStatus.SecurityLevelError,
          contentStatus: SecurityStatus.ContentStatusNormal,
          certStatus: SecurityStatus.CertStatusExpired,
          securityLevelSignals: 1, contentStatusSignals: 0,
          certStatusSignals: 1, certificateSignals: 1,
          certificate: true
        },
        {
          url: "https://localhost:4445/empty.html",
          securityLevel: SecurityStatus.SecurityLevelError,
          contentStatus: SecurityStatus.ContentStatusNormal,
          certStatus: SecurityStatus.CertStatusAuthorityInvalid,
          securityLevelSignals: 1, contentStatusSignals: 0,
          certStatusSignals: 1, certificateSignals: 1,
          certificate: true
        },
        {
          url: "https://localhost:4446/empty.html",
          securityLevel: SecurityStatus.SecurityLevelError,
          contentStatus: SecurityStatus.ContentStatusNormal,
          certStatus: SecurityStatus.CertStatusBadIdentity,
          securityLevelSignals: 1, contentStatusSignals: 0,
          certStatusSignals: 1, certificateSignals: 1,
          certificate: true
        },
        {
          url: "https://localhost:4443/tst_SecurityStatus_display_broken_subresource.html",
          securityLevel: SecurityStatus.SecurityLevelError,
          contentStatus: SecurityStatus.ContentStatusRanInsecure,
          certStatus: SecurityStatus.CertStatusOk,
          securityLevelSignals: 2, contentStatusSignals: 1,
          certStatusSignals: 0, certificateSignals: 1,
          certificate: true
        },
        {
          url: "https://localhost:4443/tst_SecurityStatus_run_broken_subresource.html",
          securityLevel: SecurityStatus.SecurityLevelError,
          contentStatus: SecurityStatus.ContentStatusRanInsecure,
          certStatus: SecurityStatus.CertStatusOk,
          securityLevelSignals: 2, contentStatusSignals: 1,
          certStatusSignals: 0, certificateSignals: 1,
          certificate: true
        },
        {
          url: "https://localhost:4443/tst_SecurityStatus_display_insecure_content.html",
          securityLevel: SecurityStatus.SecurityLevelWarning,
          contentStatus: SecurityStatus.ContentStatusDisplayedInsecure,
          certStatus: SecurityStatus.CertStatusOk,
          securityLevelSignals: 2, contentStatusSignals: 1,
          certStatusSignals: 0, certificateSignals: 1,
          certificate: true
        },
        {
          url: "https://localhost:4443/tst_SecurityStatus_run_insecure_content.html",
          securityLevel: SecurityStatus.SecurityLevelError,
          contentStatus: SecurityStatus.ContentStatusRanInsecure | SecurityStatus.ContentStatusDisplayedInsecure,
          certStatus: SecurityStatus.CertStatusOk,
          securityLevelSignals: 2, contentStatusSignals: 2,
          certStatusSignals: 0, certificateSignals: 1,
          certificate: true
        }
      ];
    }

    // This test loads content over SSL with various errors to check the behaviour
    // of the API. Each test uses a new WebView because an error permanently marks
    // a host / process combination as having ran insecure content. This can only
    // be cleared by a process swap
    function test_SecurityStatus1_main(data) {
      var webView = create_webview();

      webView.url = data.url;
      verify(webView.waitForLoadSucceeded());

      compare(webView.securityStatus.securityLevel, data.securityLevel);
      compare(webView.securityStatus.contentStatus, data.contentStatus);
      compare(webView.securityStatus.certStatus, data.certStatus);
      compare(securityLevelSpy.count, data.securityLevelSignals);
      compare(contentStatusSpy.count, data.contentStatusSignals);
      compare(certStatusSpy.count, data.certStatusSignals);
      compare(!!webView.securityStatus.certificate, data.certificate);

      var certificate = webView.securityStatus.certificate;
      var obs = OxideTestingUtils.createDestructionObserver(certificate);

      webView.url = "http://localhost:8080/empty.html";
      verify(webView.waitForLoadSucceeded());

      compare(webView.securityStatus.securityLevel, SecurityStatus.SecurityLevelNone);
      compare(webView.securityStatus.contentStatus, SecurityStatus.ContentStatusNormal);
      compare(webView.securityStatus.certStatus, SecurityStatus.CertStatusOk);
      verify(!webView.securityStatus.certificate);
      if (data.certificate) {
        verify(obs.destroyed);
      }

      webView.destroy();
    }

    property var cert_data: [
      { id: 1, parent: 5,
        serial: "00863486245ae549b6", startDate: 13054686690000, expiryDate: 21694600290000,
        fingerprint: "b354a8e3d1359447ec719e7a03b42cef379a4cc1", expired: false,
        subject: {O: "Oxide", CN: "localhost", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"},
        issuer: {O: "Canonical Ltd", CN: "Oxide Root CA", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"}
      },
      { id: 2, parent: null,
        serial: "00efc9a8e1a7a14969", startDate: 13023211916000, expiryDate: 13023298316000,
        fingerprint: "df17c8da033e2d5ed64d1f187fdf419e1fc68e47", expired: true,
        subject: {O: "Oxide", CN: "localhost", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"},
        issuer: {O: "Canonical Ltd", CN: "Oxide Root CA", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"}
      },
      { id: 3, parent: null,
        serial: "00d8b91a89244cc2bd", startDate: 13054749416000, expiryDate: 21694663016000,
        fingerprint: "f0357f544e27adaa51211663a28cc8d64b057e63", expired: false,
        subject: {O: "Oxide", CN: "localhost", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"},
        issuer: {O: "Oxide", CN: "localhost", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"}
      },
      { id: 4, parent: null,
        serial: "00b3ebd987ee2d02cc", startDate: 13054749837000, expiryDate: 21694663437000,
        fingerprint: "89c5760286e897ad32b9dd500d70755e1e026588", expired: false,
        subject: {O: "Oxide", CN: "foo.bar", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"},
        issuer: {O: "Canonical Ltd", CN: "Oxide Root CA", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"}
      },
      { id: 5, parent: null,
        serial: "00b2e64281f82a21c0", startDate: 13054686336000, expiryDate: 21694599936000,
        fingerprint: "f8ad76468322c78acbeb0845b3cdbe07138f32c4", expired: false,
        subject: {O: "Canonical Ltd", CN: "Oxide Root CA", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"},
        issuer: {O: "Canonical Ltd", CN: "Oxide Root CA", L: "Solihull", OU: undefined, C: "UK", ST: "West Midlands"}
      }
    ]

    function cert_data_from_id(id) {
      if (id === null) {
        return null;
      }

      for (var i = 0; i < cert_data.length; ++i) {
        if (cert_data[i].id == id) {
          return cert_data[i];
        }
      }

      return null;
    }

    function test_SecurityStatus2_certificate_data() {
      return [
        { url: "https://localhost:4443/empty.html", cert: 1 },
        { url: "https://localhost:4444/empty.html", cert: 2 },
        { url: "https://localhost:4445/empty.html", cert: 3 },
        { url: "https://localhost:4446/empty.html", cert: 4 }
      ];
    }

    function test_SecurityStatus2_certificate(data) {
      var webView = create_webview();

      webView.url = data.url;
      verify(webView.waitForLoadSucceeded());

      var certificate = webView.securityStatus.certificate;
      var cdata = cert_data_from_id(data.cert);

      while (cdata) {
        compare(certificate.serialNumber, cdata.serial);
        compare(certificate.effectiveDate.getTime(), cdata.startDate);
        compare(certificate.expiryDate.getTime(), cdata.expiryDate);
        compare(certificate.fingerprintSHA1, cdata.fingerprint);
        compare(certificate.isExpired, cdata.expired);

        compare(certificate.getSubjectInfo(SslCertificate.PrincipalAttrOrganizationName)[0], cdata.subject.O);
        compare(certificate.getSubjectInfo(SslCertificate.PrincipalAttrCommonName)[0], cdata.subject.CN);
        compare(certificate.getSubjectInfo(SslCertificate.PrincipalAttrLocalityName)[0], cdata.subject.L);
        compare(certificate.getSubjectInfo(SslCertificate.PrincipalAttrOrganizationUnitName)[0], cdata.subject.OU);
        compare(certificate.getSubjectInfo(SslCertificate.PrincipalAttrCountryName)[0], cdata.subject.C);
        compare(certificate.getSubjectInfo(SslCertificate.PrincipalAttrStateOrProvinceName)[0], cdata.subject.ST);

        compare(certificate.getIssuerInfo(SslCertificate.PrincipalAttrOrganizationName)[0], cdata.issuer.O);
        compare(certificate.getIssuerInfo(SslCertificate.PrincipalAttrCommonName)[0], cdata.issuer.CN);
        compare(certificate.getIssuerInfo(SslCertificate.PrincipalAttrLocalityName)[0], cdata.issuer.L);
        compare(certificate.getIssuerInfo(SslCertificate.PrincipalAttrOrganizationUnitName)[0], cdata.issuer.OU);
        compare(certificate.getIssuerInfo(SslCertificate.PrincipalAttrCountryName)[0], cdata.issuer.C);
        compare(certificate.getIssuerInfo(SslCertificate.PrincipalAttrStateOrProvinceName)[0], cdata.issuer.ST);

        certificate = certificate.issuer;
        cdata = cert_data_from_id(cdata.parent);
      }

      verify(!certificate);
    }
  }
}
