import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "certificateError"
  }

  SignalSpy {
    id: cancelSpy
    signalName: "cancelled"
  }

  property var certError: null

  onCertificateError: {
    if (certError) {
      error.allow();
      return;
    }
    certError = error;
  }

  TestCase {
    id: test
    name: "CertificateError"
    when: windowShown

    function init() {
      spy.clear();
      cancelSpy.clear();
      certError = null;
      webView.clearLoadEventCounters();
    }

    function test_CertificateError1_denials_data() {
      function _verify_1() {
        // This verifies that either there is no document or it is an error page
        try {
          comare(webView.getTestApi().documentURI, "data:text/html,chromewebdata");
        } catch(e) {
          compare(e.error, ScriptMessageRequest.ErrorDestinationNotFound);
        }
      }

      function _verify_2() {
        // This verifies that either there is no document or it is an error page
        try {
          compare(webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).documentURI,
                  "data:text/html,chromewebdata");
        } catch(e) {
          compare(e.error, ScriptMessageRequest.ErrorDestinationNotFound);
        }
      }

      function _verify_3() {
        var colour = webView.getTestApi().evaluateCode("
var elem = document.getElementById(\"foo\");
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true);
        compare(colour, "rgb(0, 0, 0)");
      }

      function _verify_4() {
        var colour = webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode("
var elem = document.getElementById(\"foo\");
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true);
        compare(colour, "rgb(0, 0, 0)");
      }

      // FIXME: Test overridable and strictEnforcement properties
      return [
        {
          loadUrl: "https://expired.testsuite/tst_CertificateError_broken_iframe.html",
          url: "https://expired.testsuite/tst_CertificateError_broken_iframe.html",
          mainframe: true, subresource: false, overridable: true,
          strictEnforcement: false, error: CertificateError.ErrorExpired,
          certificate: "df17c8da033e2d5ed64d1f187fdf419e1fc68e47",
          verifyFunc: _verify_1
        },
        {
          loadUrl: "https://testsuite/tst_CertificateError_broken_iframe.html",
          url: "https://selfsigned.testsuite/empty.html",
          mainframe: false, subresource: false, overridable: false,
          strictEnforcement: false, error: CertificateError.ErrorAuthorityInvalid,
          certificate: "f0357f544e27adaa51211663a28cc8d64b057e63",
          verifyFunc: _verify_2
        },
        {
          loadUrl: "https://testsuite/tst_CertificateError_broken_subresource.html",
          url: "https://badidentity.testsuite/tst_CertificateError_broken_subresource.css",
          mainframe: true, subresource: true, overridable: false,
          strictEnforcement: false, error: CertificateError.ErrorBadIdentity,
          certificate: "89c5760286e897ad32b9dd500d70755e1e026588",
          verifyFunc: _verify_3
        },
        {
          loadUrl: "https://testsuite/tst_CertificateError_broken_subresource_in_iframe.html",
          url: "https://badidentity.testsuite/tst_CertificateError_broken_subresource.css",
          mainframe: false, subresource: true, overridable: false,
          strictEnforcement: false, error: CertificateError.ErrorBadIdentity,
          certificate: "89c5760286e897ad32b9dd500d70755e1e026588",
          verifyFunc: _verify_4
        }
      ];
    }

    function test_CertificateError1_denials(data) {
      webView.url = data.loadUrl;
      spy.wait();

      compare(certError.url, data.url);
      verify(!certError.isCancelled);
      compare(certError.isMainFrame, data.mainframe);
      compare(certError.isSubresource, data.subresource);
      compare(certError.overridable, data.overridable);
      compare(certError.strictEnforcement, data.strictEnforcement);
      compare(certError.certError, data.error);
      compare(certError.certificate.fingerprintSHA1, data.certificate);

      certError.deny();

      if (data.mainframe && !data.subresource) {
        verify(webView.waitForLoadStopped());
        compare(webView.loadsFailedCount, 0);
        compare(webView.loadsStoppedCount, 1);
        compare(webView.loadsSucceededCount, 0);
      } else {
        verify(webView.waitForLoadSucceeded());
        if (!data.mainframe) {
          // This is a bit of a hack, but we don't have another event we can
          // fire off for subframe loads
          wait(100);
        }
      }

      compare(webView.securityStatus.securityLevel,
              data.mainframe && !data.subresource ?
                  SecurityStatus.SecurityLevelNone : SecurityStatus.SecurityLevelSecure);
      compare(webView.securityStatus.contentStatus, SecurityStatus.ContentStatusNormal);
      compare(webView.securityStatus.certStatus, SecurityStatus.CertStatusOk);

      data.verifyFunc();
    }

    function test_CertificateError2_allow_data() {
      function _verify_1() {
        compare(webView.getTestApi().documentURI, "https://expired.testsuite/tst_CertificateError_broken_iframe.html");
      }

      function _verify_2() {
        // This verifies that either there is no document or it is an error page
        try {
          compare(webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).documentURI,
                  "data:text/html,chromewebdata");
        } catch(e) {
          compare(e.error, ScriptMessageRequest.ErrorDestinationNotFound);
        }
      }

      function _verify_3() {
        var colour = webView.getTestApi().evaluateCode("
var elem = document.getElementById(\"foo\");
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true);
        compare(colour, "rgb(0, 0, 0)");
      }

      function _verify_4() {
        var colour = webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode("
var elem = document.getElementById(\"foo\");
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true);
        compare(colour, "rgb(0, 0, 0)");
      }

      return [
        {
          loadUrl: "https://expired.testsuite/tst_CertificateError_broken_iframe.html",
          mainframe: true, overridable: true, verifyFunc: _verify_1
        },
        {
          loadUrl: "https://testsuite/tst_CertificateError_broken_iframe.html",
          mainframe: false, overridable: false, verifyFunc: _verify_2
        },
        {
          loadUrl: "https://testsuite/tst_CertificateError_broken_subresource.html",
          mainframe: false, overridable: false, verifyFunc: _verify_3
        },
        {
          loadUrl: "https://testsuite/tst_CertificateError_broken_subresource_in_iframe.html",
          mainframe: false, overridable: false, verifyFunc: _verify_4
        }
      ];
    }

    function test_CertificateError2_allow(data) {
      webView.url = data.loadUrl;
      spy.wait();

      certError.allow();

      verify(webView.waitForLoadSucceeded());

      if (data.mainframe) {
        compare(webView.loadsFailedCount, 0);
        compare(webView.loadsStoppedCount, 0);
        compare(webView.loadsSucceededCount, 1);
      } else {
        // This is a bit of a hack, but we don't have another event we can fire off
        wait(100);
      }

      data.verifyFunc();
    }

    function test_CertificateError3_cancellation() {
      webView.url = "https://expired.testsuite/tst_CertificateError_broken_iframe.html";
      spy.wait();

      cancelSpy.target = certError;

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      verify(certError.isCancelled);
      compare(cancelSpy.count, 1);

      cancelSpy.target = null;
    }
  }
}
