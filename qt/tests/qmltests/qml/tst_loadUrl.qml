import QtQuick 2.0
import QtTest 1.0
import "../../utils"

TestWebView {
  id: webView
  focus: true

  TestCase {
    id: test
    name: "WebViewLoadURL"
    when: windowShown

    function init() {
      webView.resetLoadCounters();
    }

    function test_loadUrl_data() {
      return [
        { url: Qt.resolvedUrl("http://localhost:8080/empty.html"), succeeded: 1, failed: 0 },
        { url: Qt.resolvedUrl("../../html/empty.html"), succeeded: 1, failed: 0 },
        { url: Qt.resolvedUrl("about:blank"), succeeded: 1, failed: 0 }
      ];
    }

    function test_loadUrl(data) {
      webView.url = data.url;
      verify(webView.waitForLoadSucceeded());

      compare(webView.loadsSucceededCount, data.succeeded);
      compare(webView.loadsFailedCount, data.failed);
      compare(webView.url, data.url);

      compare(webView.getTestApi().documentURI, data.url);
    }
  }
}
