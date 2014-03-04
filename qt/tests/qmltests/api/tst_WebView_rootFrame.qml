import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "rootFrameChanged"
  }

  TestCase {
    id: test
    name: "WebView_rootFrame"
    when: windowShown

    function test_WebView_rootFrame1_parent() {
      verify(webView.rootFrame, "Should always have a root frame");
      compare(OxideTestingUtils.qObjectParent(webView.rootFrame), webView,
              "The root frame should be parented to the webview");
    }

    function test_WebView_rootFrame2_undeletable() {
      var caught = false;
      try {
        webView.rootFrame.destroy();
      } catch(e) {
        caught = true;
      }

      verify(caught, "WebView.rootFrame.destroy() should have thrown");
    }

    function test_WebView_rootFrame3_creation_signal() {
      compare(spy.count, 1, "Should have had 1 signal during construction");
    }
  }
}
