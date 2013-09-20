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
      compare(webView.loading, false);

      webView.url = "http://localhost:8080/empty.html";
      verify(webView.waitForLoadSucceeded());

      compare(webView.loading, false);
      compare(loadingStateChangeCount, 2);
    }
  }
}
