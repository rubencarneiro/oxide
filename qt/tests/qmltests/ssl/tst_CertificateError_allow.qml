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

  onCertificateError: {
    error.allow();
  }

  TestCase {
    id: test
    name: "CertificateError_allow"
    when: windowShown

    function init() {
      spy.clear();
      webView.clearLoadEventCounters();
    }

    function _verify_1() {
      try {
        compare(webView.getTestApi().documentURI, "https://localhost:4444/tst_CertificateError_broken_iframe.html");
      } catch(e) {
        verify(false);
      }
    }

    function _verify_2() {
      try {
        compare(webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).documentURI, "https://localhost:4445/empty.html");
      } catch(e) {
        verify(false);
      }
    }

    function _verify_3() {
      var colour = webView.getTestApi().evaluateCode("
var elem = document.getElementById(\"foo\");
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true);
      compare(colour, "rgb(0, 128, 0)");
    }

    function _verify_4() {
      var colour = webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode("
var elem = document.getElementById(\"foo\");
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true);
      compare(colour, "rgb(0, 128, 0)");
    }

    function test_CertificateError_allow1_data() {
      return [
        {
          loadUrl: "https://localhost:4444/tst_CertificateError_broken_iframe.html",
          mainframe: true, verifyFunc: _verify_1
        },
        {
          loadUrl: "https://localhost:4443/tst_CertificateError_broken_iframe.html",
          mainframe: false, errorCount: 1, verifyFunc: _verify_2
        },
        {
          loadUrl: "https://localhost:4443/tst_CertificateError_broken_subresource.html",
          mainframe: false, errorCount: 1, verifyFunc: _verify_3
        },
        {
          // FIXME: This one generates no errors because the CSS file gets loaded from cache
          loadUrl: "https://localhost:4443/tst_CertificateError_broken_subresource_in_iframe.html",
          mainframe: false, errorCount: 0, verifyFunc: _verify_4
        }
      ];
    }

    function test_CertificateError_allow1(data) {
      webView.url = data.loadUrl;

      if (data.mainframe) {
        verify(webView.waitForLoadSucceeded());
        compare(webView.loadsFailedCount, 0);
        compare(webView.loadsStoppedCount, 0);
        compare(webView.loadsSucceededCount, 1);
      } else {
        webView.waitFor(function() { return spy.count == data.errorCount; });
        // This is a bit of a hack, but we don't have another event we can fire off
        wait(100);
      }

      data.verifyFunc();
    }
  }
}
