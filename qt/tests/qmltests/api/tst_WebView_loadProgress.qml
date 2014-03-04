import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property int inconsistentState: 0
  onLoadingChanged: {
    if (!loading && (loadProgress < 100)) {
      inconsistentState++
    }
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "loadProgressChanged"
  }

  TestCase {
    name: "WebView_loadProgress"
    when: windowShown

    function test_WebView_loadProgress() {
      compare(webView.loadProgress, 0,
              "WebView.loadProgress should initially be 0%");

      webView.url = "http://localhost:8080/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      compare(webView.loadProgress, 100,
              "WebView.loadProgress should be 100% after we finish loading");
      verify(spy.count > 0,
             "WebView.loadProgress should have changed during the load");
      compare(webView.inconsistentState, 0,
              "WebView.loadProgress should always be 100% when not loading");
    }
  }
}
