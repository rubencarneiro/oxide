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

    function _test_1_init() {
      webView.getTestApi().evaluateCode("
var url = window.location + \"#\";
var i = 0;
while (i++ < 100000) {
  url = url + \"AAAAAAAAAAAAAAAAAAAAAAAAA\";
}
window.location = url;", false);
    }

    function _test_2_init() {
      webView.getTestApi().evaluateCode("window.location = window.location + \"?foo=bar\";", false);
    }

    function _test_3_init() {
      webView.getTestApi().evaluateCode("
var url = window.location + \"?foo=bar#\";
var i = 0;
while (i++ < 100000) {
  url = url + \"AAAAAAAAAAAAAAAAAAAAAAAAA\";
}
window.location = url;", false);
    }

    function _test_4_init() {
      webView.getTestApi().evaluateCode("
var url = window.location + \"?\";
var i = 0;
while (i++ < 100000) {
  url = url + \"foo=bar&foo=bar&foo=bar&\";
}
window.location = url;", false);
    }

    function init() {
      webView.lastOverrideUrl = null;

      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.url = "http://localhost:8080/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }

    function test_bug1349510_data() {
      return [
        { prep: null, url: "http://localhost:8080/empty.html" },
        { prep: _test_1_init, url: "http://localhost:8080/empty.html" },
        { prep: _test_2_init, url: "http://localhost:8080/empty.html?foo=bar" }
// FIXME: Disabled because they trigger a renderer abort when sending FrameHostMsg_OpenURL
//        { prep: _test_3_init, url: "http://localhost:8080/empty.html?foo=bar" },
//        { prep: _test_4_init, url: "http://localhost:8080/empty.html" }
      ];
    }

    function test_bug1349510(data) {
      if (data.prep) {
        data.prep();
      }

      compare(webView.getTestApi().evaluateCode("navigator.userAgent", false),
              "Foo");
      compare(webView.lastOverrideUrl, data.url);
    }
  }
}
