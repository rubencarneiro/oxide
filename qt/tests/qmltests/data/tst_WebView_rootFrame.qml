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

    SignalSpy {
      id: spy
      target: webView
      signalName: "rootFrameChanged"
    }

    function test_WebView_rootFrame1() {
      verify(!webView.rootFrame, "Shouldn't have a frame yet");

      webView.url = "http://localhost:8080/tst_WebView_rootFrame.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(spy.count, 1, "Should have a root frame");
      verify(webView.rootFrame, "Should have a root frame");

      var root = webView.rootFrame;

      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(spy.count, 2, "Should have a new root frame");
      verify(webView.rootFrame, "Should have a root frame");

      compare(root.sendMessage, undefined, "The old root frame should no longer exist");

      webView.goBack();
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(spy.count, 3, "Should have the old root frame back again now");
      verify(webView.rootFrame, "Should have a root frame");
    }
  }
}
