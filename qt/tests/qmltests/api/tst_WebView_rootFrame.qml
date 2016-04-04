import QtQuick 2.0
import QtTest 1.0
import Oxide.testsupport 1.0

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

  Component {
    id: webViewFactory
    TestWebView {
      property var rootFrame2: undefined
      property bool initRootFrame2: false
      onInitRootFrame2Changed: rootFrame2 = rootFrame
    }
  }

  TestCase {
    id: test
    name: "WebView_rootFrame"
    when: windowShown

    function test_WebView_rootFrame1_parent() {
      verify(webView.rootFrame, "Should always have a root frame");
      verify(!webView.rootFrame.parent, "Root frame should have no parent");
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

    // Verify that accessing WebView.rootFrame before a view is unitialized
    // doesn't crash
    function test_WebView_rootFrame4_uninitialized() {
      var view = webViewFactory.createObject(null, { initRootFrame2: true });
      compare(view.rootFrame2, null);
    }
  }
}
