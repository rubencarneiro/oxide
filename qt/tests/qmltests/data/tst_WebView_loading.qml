import QtQuick 2.0
import QtTest 1.0
import "../../utils"

TestWebView {
  id: webView
  focus: true

  SignalSpy {
    id: spy
    target: webView
    signalName: "loadingChanged"
  }

  TestCase {
    id: test
    name: "WebView_loading"
    when: windowShown

    function test_WebView_loading() {
      compare(webView.loading, false,
              "WebView.loading should be false before we start loading");

      webView.url = "http://localhost:8080/tst_WebView_loading.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      compare(webView.loading, false,
              "WebView.loading should be false after we finish loading");
      compare(spy.count, 2,
              "WebView.loading should have changed twice during the load");
    }
  }
}
