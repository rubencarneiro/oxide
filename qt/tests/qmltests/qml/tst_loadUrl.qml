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

    function test_loadUrl1_data() {
      return [
        { url: "http://localhost:8080/empty.html", succeeded: 1, failed: 0 },
        { url: Qt.resolvedUrl("../../html/empty.html"), succeeded: 1, failed: 0 },
        { url: "about:blank", succeeded: 1, failed: 0 }
      ];
    }

    function test_loadUrl1(data) {
      webView.url = data.url;
      verify(webView.waitForLoadSucceeded());

      compare(webView.loadsSucceededCount, data.succeeded);
      compare(webView.loadsFailedCount, data.failed);
      compare(webView.url, data.url);

      compare(webView.getTestApi().documentURI, data.url);
    }

    function test_loadUrl2_ignoreEmpty() {
      var url = "http://localhost:8080/empty.html";

      webView.url = url;
      verify(webView.waitForLoadSucceeded());
      compare(webView.loadsStartedCount, 1);
      compare(webView.loadsSucceededCount, 1);
      compare(webView.loadsFailedCount, 0);
      compare(webView.url, url);

      webView.url = "";
      wait(1000);
      compare(webView.loadsStartedCount, 1);
      compare(webView.url, url);
    }
  }
}
