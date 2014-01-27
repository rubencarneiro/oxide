import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  TestCase {
    id: test
    name: "WebView_rootFrame"
    when: windowShown

    function test_WebView_rootFrame1() {
      verify(webView.rootFrame, "Should always have a root frame");
      compare(Utils.qObjectParent(webView.rootFrame), webView,
              "The root frame should be parented to the webview");
    }
  }
}
