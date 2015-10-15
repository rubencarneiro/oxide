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

  SignalSpy {
    id: urlSpy
    target: webView
    signalName: "urlChanged"
  }

  SignalSpy {
    id: titleSpy
    target: webView
    signalName: "titleChanged"
  }

  property var certError: null

  onCertificateError: {
    if (certError) {
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
      cancelSpy.target = null;
      cancelSpy.clear();
      urlSpy.clear();
      titleSpy.clear();
      certError = null;
      webView.clearLoadEventCounters();
    }

    function test_CertificateError1_subresource_deny_data() {
      function _verify_1() {
        // This verifies that either there is no document or it is an error page
        try {
          compare(webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).documentURI,
                  "data:text/html,chromewebdata");
        } catch(e) {
          compare(e.error, ScriptMessageRequest.ErrorDestinationNotFound);
        }
      }

      function _verify_2() {
        var colour = webView.getTestApi().evaluateCode("
var elem = document.getElementById(\"foo\");
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true);
        compare(colour, "rgb(0, 0, 0)");
      }

      function _verify_3() {
        var colour = webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode("
var elem = document.getElementById(\"foo\");
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true);
        compare(colour, "rgb(0, 0, 0)");
      }

      // FIXME: Test overridable and strictEnforcement properties
      return [
        {
          loadUrl: "https://testsuite/tst_CertificateError_broken_iframe.html",
          url: "https://selfsigned.testsuite/empty.html",
          mainframe: false, subresource: false,
          strictEnforcement: false, error: CertificateError.ErrorAuthorityInvalid,
          certificate: "f0357f544e27adaa51211663a28cc8d64b057e63",
          verifyFunc: _verify_1
        },
        {
          loadUrl: "https://testsuite/tst_CertificateError_broken_subresource.html",
          url: "https://badidentity.testsuite/tst_CertificateError_broken_subresource.css",
          mainframe: true, subresource: true,
          strictEnforcement: false, error: CertificateError.ErrorBadIdentity,
          certificate: "89c5760286e897ad32b9dd500d70755e1e026588",
          verifyFunc: _verify_2
        },
        {
          loadUrl: "https://testsuite/tst_CertificateError_broken_subresource_in_iframe.html",
          url: "https://badidentity.testsuite/tst_CertificateError_broken_subresource.css",
          mainframe: false, subresource: true,
          strictEnforcement: false, error: CertificateError.ErrorBadIdentity,
          certificate: "89c5760286e897ad32b9dd500d70755e1e026588",
          verifyFunc: _verify_3
        }
      ];
    }

    // This tests CertificateError for various subresource errors, and verifies
    // that those subresources are always blocked
    function test_CertificateError1_subresource_deny(data) {
      webView.url = data.loadUrl;
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      if (!data.mainframe) {
        // This is a bit of a hack, but we don't have another event we can
        // fire off for subframe loads
        wait(100);
      }

      compare(certError.url, data.url);
      verify(!certError.isCancelled);
      compare(certError.isMainFrame, data.mainframe);
      compare(certError.isSubresource, data.subresource);
      compare(certError.overridable, false);
      compare(certError.strictEnforcement, data.strictEnforcement);
      compare(certError.certError, data.error);
      compare(certError.certificate.fingerprintSHA1, data.certificate);

      compare(webView.url, data.loadUrl);

      compare(webView.securityStatus.securityLevel,SecurityStatus.SecurityLevelSecure);
      compare(webView.securityStatus.contentStatus, SecurityStatus.ContentStatusNormal);
      compare(webView.securityStatus.certStatus, SecurityStatus.CertStatusOk);

      data.verifyFunc();

      // There's nothing to deny, but it shouldn't crash
      certError.deny();
    }

    function test_CertificateError2_mainframe_deny_data() {
      function _verify_1() {
        // This verifies that the old document still exists
        compare(webView.getTestApi().documentURI, "https://testsuite/tst_CertificateError_initial.html");
      }

      // FIXME: Test non-overridable errors too
      return [
        {
          loadUrl: "https://expired.testsuite/empty.html",
          overridable: true,
          strictEnforcement: false, error: CertificateError.ErrorExpired,
          certificate: "df17c8da033e2d5ed64d1f187fdf419e1fc68e47",
          verifyFunc: _verify_1
        },
      ];

    }

    // This test CertificateError for main frame errors, and verifies that
    // calling deny() blocks them
    function test_CertificateError2_mainframe_deny(data) {
      webView.url = "https://testsuite/tst_CertificateError_initial.html";
      verify(webView.waitForLoadSucceeded());

      compare(webView.title, "Test");

      webView.clearLoadEventCounters();
      titleSpy.clear();

      webView.url = data.loadUrl;
      spy.wait();
      titleSpy.wait();
      if (!data.overridable) {
        verify(webView.waitForLoadStopped());
      }

      compare(certError.url, data.loadUrl);
      verify(!certError.isCancelled);
      compare(certError.isMainFrame, true);
      compare(certError.isSubresource, false);
      compare(certError.overridable, data.overridable);
      compare(certError.strictEnforcement, data.strictEnforcement);
      compare(certError.certError, data.error);
      compare(certError.certificate.fingerprintSHA1, data.certificate);

      compare(webView.loadsStartedCount, 1);
      compare(webView.loadsCommittedCount, 0);
      compare(webView.loadsFailedCount, 0);
      compare(webView.loadsSucceededCount, 0);

      // WebView.url should indicate the URL of the failing resource even for
      // non-overridable errors, as we show a placeholder transient page
      compare(webView.url, data.loadUrl);
      compare(webView.title, data.loadUrl);

      data.verifyFunc();

      urlSpy.clear();
      titleSpy.clear();

      certError.deny();

      if (data.overridable) {
        verify(webView.waitForLoadStopped());
      }
      urlSpy.wait();
      titleSpy.wait();

      compare(webView.url, "https://testsuite/tst_CertificateError_initial.html");
      compare(webView.title, "Test");

      data.verifyFunc();
    }

    function test_CertificateError3_subresource_allow_data() {
      function _verify_1() {
        // This verifies that either there is no document or it is an error page
        try {
          compare(webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).documentURI,
                  "data:text/html,chromewebdata");
        } catch(e) {
          compare(e.error, ScriptMessageRequest.ErrorDestinationNotFound);
        }
      }

      function _verify_2() {
        var colour = webView.getTestApi().evaluateCode("
var elem = document.getElementById(\"foo\");
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true);
        compare(colour, "rgb(0, 0, 0)");
      }

      function _verify_3() {
        var colour = webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode("
var elem = document.getElementById(\"foo\");
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true);
        compare(colour, "rgb(0, 0, 0)");
      }

      return [
        {
          loadUrl: "https://testsuite/tst_CertificateError_broken_iframe.html",
          verifyFunc: _verify_1
        },
        {
          loadUrl: "https://testsuite/tst_CertificateError_broken_subresource.html",
          verifyFunc: _verify_2
        },
        {
          loadUrl: "https://testsuite/tst_CertificateError_broken_subresource_in_iframe.html",
          verifyFunc: _verify_3
        }
      ];
    }

    // Verifies that calling allow() on subresource errors cannot override
    // them
    function test_CertificateError3_subresource_allow(data) {
      webView.url = data.loadUrl;
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      certError.allow();

      // This is a bit of a hack, but we don't have another event we can fire off
      wait(100);

      data.verifyFunc();
    }

    function test_CertificateError4_mainframe_allow_data() {
      function _verify_1() {
        compare(webView.getTestApi().documentURI, "https://expired.testsuite/empty.html");
      }

      // FIXME: Test non-overridable errors too
      return [
        {
          loadUrl: "https://expired.testsuite/empty.html",
          overridable: true, verifyFunc: _verify_1
        },
      ];
    }

    // Verifies that calling allow() on overridable main frame errors allows
    // the page to continue loading
    function test_CertificateError4_mainframe_allow(data) {
      webView.url = "https://testsuite/tst_CertificateError_initial.html";
      verify(webView.waitForLoadSucceeded());

      webView.clearLoadEventCounters();

      webView.url = data.loadUrl;
      spy.wait();
      if (!data.overridable) {
        verify(webView.waitForLoadStopped());
      }

      compare(webView.loadsStartedCount, 1);
      compare(webView.loadsCommittedCount, 0);
      compare(webView.loadsFailedCount, 0);
      compare(webView.loadsSucceededCount, 0);

      certError.allow();

      if (data.overridable) {
        verify(webView.waitForLoadSucceeded());
      }

      data.verifyFunc();
    }

    function test_CertificateError5_cancellation_browser_initiated_navigation_data() {
      // Test non-overridable too
      return [
        { loadUrl: "https://badidentity.testsuite/empty.html", overridable: true }
      ];
    }

    // Verify that main frame errors are cancelled on a browser-initiated navigation
    function test_CertificateError5_cancellation_browser_initiated_navigation(data) {
      webView.url = "https://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.url = data.loadUrl;
      spy.wait();
      if (!data.overridable) {
        verify(webView.waitForLoadStopped());
      }

      cancelSpy.target = certError;
      verify(!certError.isCancelled);

      webView.clearLoadEventCounters();

      webView.url = "https://foo.testsuite/empty.html";
      verify(webView.waitForLoadCommitted());

      compare(cancelSpy.count, 1);
      verify(certError.isCancelled);

      verify(webView.waitForLoadSucceeded());
      compare(webView.url, "https://foo.testsuite/empty.html");
    }

    function test_CertificateError6_cancellation_history_navigation_data() {
      return test_CertificateError5_cancellation_browser_initiated_navigation_data();
    }

    function test_CertificateError6_cancellation_history_navigation(data) {
      webView.url = "https://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.url = data.loadUrl;
      spy.wait();
      if (!data.overridable) {
        verify(webView.waitForLoadStopped());
      }

      cancelSpy.target = certError;
      verify(!certError.isCancelled);

      urlSpy.clear();

      // FIXME(chrisccoulson): The test fails without the delay - cancellation
      //  never happens. I suspect this is because we go back before getting
      //  the commit from the renderer, which means that the interstitial is
      //  not attached to the WebContents and DontProceed doesn't get called
      wait(100);

      webView.goBack();
      // There's no new load here, so just wait for the URL to change back
      urlSpy.wait();
      cancelSpy.wait();

      compare(cancelSpy.count, 1);
      verify(certError.isCancelled);
      compare(webView.url, "https://testsuite/empty.html");
    }

    function test_CertificateError7_content_initiated_navigation_data() {
      return test_CertificateError5_cancellation_browser_initiated_navigation_data();
    }

    function test_CertificateError7_content_initiated_navigation(data) {
      webView.url = "https://testsuite/tst_CertificateError_initial.html";
      verify(webView.waitForLoadSucceeded());

      urlSpy.clear();
      titleSpy.clear();
      webView.clearLoadEventCounters();

      webView.getTestApi().evaluateCode("window.location = \"" + data.loadUrl + "\";");
      spy.wait();
      urlSpy.wait();
      titleSpy.wait();
      if (!data.overridable) {
        verify(webView.waitForLoadStopped());
      }

      compare(webView.loadsStartedCount, 1);
      compare(webView.loadsCommittedCount, 0);
      compare(webView.loadsFailedCount, 0);
      compare(webView.loadsSucceededCount, 0);

      compare(webView.url, data.loadUrl);
      compare(webView.loadsStartedCount, 1);
      compare(webView.loadsCommittedCount, 0);
      compare(webView.loadsFailedCount, 0);
      compare(webView.loadsSucceededCount, 0);
      compare(webView.title, data.loadUrl);
    }
  }
}
