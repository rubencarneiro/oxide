import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property var lastOverrideUrl: null

  context.userAgentOverrideDelegate: WebContextDelegateWorker {
    source: Qt.resolvedUrl("tst_bug1349510.js")
    onMessage: {
      webView.lastOverrideUrl = message.url;
    }
  }

  TestCase {
    name: "bug1349510"
    when: windowShown

    function _append_fragment() {
      webView.getTestApi().evaluateCode("
var url = window.location + \"#\";
var i = 0;
while (i++ < 100000) {
  url = url + \"AAAAAAAAAAAAAAAAAAAAAAAAA\";
}
window.location = url;", false);
    }

    function init() {
      webView.lastOverrideUrl = null;

      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

    }

    function test_bug1349510_data() {
      return [
        { prep: null, loadUrl: "http://localhost:8080/empty.html" },
        { prep: _append_fragment, loadUrl: "http://localhost:8080/empty.html" },
        { prep: null, loadUrl: "http://localhost:8080/empty.html?foo=bar" },
        { prep: _append_fragment, loadUrl: "http://localhost:8080/empty.html?foo=bar" },
        { prep: null, loadUrl: "http://localhost:8080/empty.html#AAAAAAAAAAA", overrideUrl: "http://localhost:8080/empty.html" },
        { prep: null, loadUrl: "http://foo:password@localhost:8080/empty.html#AAAAAAA", overrideUrl: "http://localhost:8080/empty.html" }
      ];
    }

    function test_bug1349510(data) {
      webView.url = data.loadUrl;
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      if (data.prep) {
        data.prep();
      }

      compare(webView.getTestApi().evaluateCode("navigator.userAgent", false),
              "Foo");
      var overrideUrl = data.overrideUrl || data.loadUrl;
      compare(webView.lastOverrideUrl, overrideUrl);
    }
  }
}
