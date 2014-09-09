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

    function test_SecurityStatus1_data() {
      // XXX: How do we test that EV works?
      return [
        {
          url: "http://localhost:8080/empty.html",
          securityLevel: SecurityStatus.SecurityLevelNone,
          contentStatus: SecurityStatus.ContentStatusNormal,
          certStatus: SecurityStatus.CertStatusOk,
          securityLevelSignals: [0,0], contentStatusSignals: [0,0],
          certStatusSignals: [0,0], certificateSignals: [0,0],
          certificate: false
        },
        {
          url: "https://localhost:4443/empty.html",
          securityLevel: SecurityStatus.SecurityLevelSecure,
          contentStatus: SecurityStatus.ContentStatusNormal,
          certStatus: SecurityStatus.CertStatusOk,
          securityLevelSignals: [1,2], contentStatusSignals: [0,0],
          certStatusSignals: [0,0], certificateSignals: [1,2],
          certificate: true
        },
        {
          url: "https://localhost:4444/empty.html",
          securityLevel: SecurityStatus.SecurityLevelError,
          contentStatus: SecurityStatus.ContentStatusNormal,
          certStatus: SecurityStatus.CertStatusExpired,
          securityLevelSignals: [1,2], contentStatusSignals: [0,0],
          certStatusSignals: [1,2], certificateSignals: [1,2],
          certificate: true
        },
        {
          url: "https://localhost:4445/empty.html",
          securityLevel: SecurityStatus.SecurityLevelError,
          contentStatus: SecurityStatus.ContentStatusNormal,
          certStatus: SecurityStatus.CertStatusAuthorityInvalid,
          securityLevelSignals: [1,2], contentStatusSignals: [0,0],
          certStatusSignals: [1,2], certificateSignals: [1,2],
          certificate: true
        },
        {
          url: "https://localhost:4446/empty.html",
          securityLevel: SecurityStatus.SecurityLevelError,
          contentStatus: SecurityStatus.ContentStatusNormal,
          certStatus: SecurityStatus.CertStatusBadIdentity,
          securityLevelSignals: [1,2], contentStatusSignals: [0,0],
          certStatusSignals: [1,2], certificateSignals: [1,2],
          certificate: true
        },
        {
          url: "https://localhost:4443/tst_SecurityStatus_display_broken_subresource.html",
          securityLevel: SecurityStatus.SecurityLevelError,
          contentStatus: SecurityStatus.ContentStatusRanInsecure,
          certStatus: SecurityStatus.CertStatusOk,
          securityLevelSignals: [2,3], contentStatusSignals: [1,2],
          certStatusSignals: [0,0], certificateSignals: [1,2],
          certificate: true
        },
        {
          url: "https://localhost:4443/tst_SecurityStatus_run_broken_subresource.html",
          securityLevel: SecurityStatus.SecurityLevelError,
          contentStatus: SecurityStatus.ContentStatusRanInsecure,
          certStatus: SecurityStatus.CertStatusOk,
          securityLevelSignals: [2,3], contentStatusSignals: [1,2],
          certStatusSignals: [0,0], certificateSignals: [1,2],
          certificate: true
        },
        {
          url: "https://localhost:4443/tst_SecurityStatus_display_insecure_content.html",
          securityLevel: SecurityStatus.SecurityLevelWarning,
          contentStatus: SecurityStatus.ContentStatusDisplayedInsecure,
          certStatus: SecurityStatus.CertStatusOk,
          securityLevelSignals: [2,3], contentStatusSignals: [1,2],
          certStatusSignals: [0,0], certificateSignals: [1,2],
          certificate: true
        },
        {
          url: "https://localhost:4443/tst_SecurityStatus_run_insecure_content.html",
          securityLevel: SecurityStatus.SecurityLevelError,
          contentStatus: SecurityStatus.ContentStatusRanInsecure | SecurityStatus.ContentStatusDisplayedInsecure,
          certStatus: SecurityStatus.CertStatusOk,
          securityLevelSignals: [2,3], contentStatusSignals: [2,3],
          certStatusSignals: [0,0], certificateSignals: [1,2],
          certificate: true
        }
      ];
    }

    // This test loads content over SSL with various errors to check the behaviour
    // of the API. Each test uses a new WebView because an error permanently marks
    // a host / process combination as having ran insecure content. This can only
    // be cleared by a process swap
    function test_SecurityStatus1(data) {
      var webView = create_webview();

      webView.url = data.url;
      verify(webView.waitForLoadSucceeded());

      compare(webView.securityStatus.securityLevel, data.securityLevel);
      compare(webView.securityStatus.contentStatus, data.contentStatus);
      compare(webView.securityStatus.certStatus, data.certStatus);
      compare(!!webView.securityStatus.certificate, data.certificate);
      compare(securityLevelSpy.count, data.securityLevelSignals[0]);
      compare(contentStatusSpy.count, data.contentStatusSignals[0]);
      compare(certStatusSpy.count, data.certStatusSignals[0]);
      compare(certificateSpy.count, data.certificateSignals[0]);

      var certificate = webView.securityStatus.certificate;
      var obs = OxideTestingUtils.createDestructionObserver(certificate);

      webView.url = "http://localhost:8080/empty.html";
      verify(webView.waitForLoadSucceeded());

      compare(webView.securityStatus.securityLevel, SecurityStatus.SecurityLevelNone);
      compare(webView.securityStatus.contentStatus, SecurityStatus.ContentStatusNormal);
      compare(webView.securityStatus.certStatus, SecurityStatus.CertStatusOk);
      verify(!webView.securityStatus.certificate);
      compare(securityLevelSpy.count, data.securityLevelSignals[1]);
      compare(contentStatusSpy.count, data.contentStatusSignals[1]);
      compare(certStatusSpy.count, data.certStatusSignals[1]);
      compare(certificateSpy.count, data.certificateSignals[1]);
      if (data.certificate) {
        verify(obs.destroyed);
      }

      webView.destroy();
    }
  }
}
