import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.9
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView

  focus: true

  width: 200
  height: 200

  TestCase {
    id: test
    name: "WebContext_doNotTrack"
    when: windowShown

    function init() {
      webView.context.doNotTrack = false
    }
 
    function test_dnt_data() {
      return [
        {expectedHeaderDnt: "0", expectedNavigatorDnt: "0", dnt: false},
        {expectedHeaderDnt: "1", expectedNavigatorDnt: "1", dnt: true}
      ]
    }

    function test_dnt(data) {
      webView.context.doNotTrack = data.dnt

      webView.url = "http://testsuite/tst_WebContext_doNotTrack.py";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().evaluateCode(
              "document.querySelector(\"#dnt\").innerHTML"),
              data.expectedHeaderDnt);
      compare(webView.getTestApi().evaluateCode(
              "window.navigator.doNotTrack && window.navigator.doNotTrack === '1' ? '1' : '0'"),
              data.expectedNavigatorDnt);
    }
  }
}
