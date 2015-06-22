import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  TestWebContext {
    id: c
  }

  context: c

  SignalSpy {
    id: spy
    target: webView
    signalName: "rootFrameChanged"
  }

  Component {
    id: webViewFactory
    TestWebView { context: c }
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
      var done = false;
      function create_view(request) {
        var created = webViewFactory.createObject(null, { request: request });
        verify(!created.rootFrame);
        created.destroy();
        done = true;
      }

      webView.newViewRequested.connect(create_view);
      webView.context.popupBlockerEnabled = false;

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("window.open(\"empty.html\");", true);
      TestUtils.waitFor(function() { return done; });

      webView.context.popupBlockerEnabled = true;
      webView.newViewRequested.disconnect(create_view);
    }
  }
}
