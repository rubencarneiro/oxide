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
    signalName: "loadProgressChanged"
  }

  TestCase {
    name: "WebView_loadProgress"
    when: windowShown

    function test_WebView_loadProgress() {
      compare(webView.loadProgress, 0,
              "WebView.loadProgress should be 0% before we start loading");

      webView.url = "http://localhost:8080/tst_WebView_loadProgress.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      compare(webView.loadProgress, 100,
              "WebView.loadProgress should be 100% after we finish loading");
      verify(spy.count > 0,
              "WebView.loadProgress should have changed during the load");
    }
  }
}
