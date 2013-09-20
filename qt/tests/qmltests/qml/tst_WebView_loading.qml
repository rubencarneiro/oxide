import QtQuick 2.0
import QtTest 1.0
import "../../utils"

TestWebView {
  id: webView
  focus: true

  property var loadingStateChangeCount: 0

  function loadingStateChanged() {
    loadingStateChangeCount++;
  }

  TestCase {
    id: test
    name: "WebView_loading"
    when: windowShown

    function test_WebView_loading() {
      compare(webView.loading, false,
              "WebView.loading should be false before we start loading");

      webView.url = "http://localhost:8080/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      compare(webView.loading, false,
              "WebView.loading should be false after we finish loading");
      compare(loadingStateChangeCount, 2,
              "WebView.loading should have changed twice during the load");
    }
  }
}
