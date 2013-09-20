import QtQuick 2.0
import QtTest 1.0
import "../../utils"

TestWebView {
  id: webView
  focus: true

  TestCase {
    id: test
    name: "WebView_loadUrl"
    when: windowShown

    function init() {
      webView.resetLoadCounters();
    }

    function test_WebView_loadUrl1_data() {
      return [
        { url: "http://localhost:8080/empty.html", succeeded: 1, failed: 0 },
        { url: Qt.resolvedUrl("../../www/empty.html"), succeeded: 1, failed: 0 },
        { url: "about:blank", succeeded: 1, failed: 0 },
        { url: "foo://bar.com", succeeded: 1, failed: 1, documentURI: "data:text/html,chromewebdata" }
      ];
    }

    function test_WebView_loadUrl1(data) {
      webView.url = data.url;
      verify(webView.waitForLoadSucceeded());

      compare(webView.loadsSucceededCount, data.succeeded);
      compare(webView.loadsFailedCount, data.failed);
      compare(webView.loadsStartedCount, data.succeeded + data.failed);
      compare(webView.url, data.url);

      if (!("documentURI" in data)) {
        data.documentURI = data.url;
      }

      compare(webView.getTestApi().documentURI, data.documentURI);
    }

    function test_WebView_loadUrl2_ignoreInvalid_data() {
      return [
        { url: "" }
      ];
    }

    function test_WebView_loadUrl2_ignoreInvalid(data) {
      var url = "http://localhost:8080/empty.html";

      webView.url = url;
      verify(webView.waitForLoadSucceeded());
      compare(webView.loadsStartedCount, 1);
      compare(webView.loadsSucceededCount, 1);
      compare(webView.loadsFailedCount, 0);
      compare(webView.url, url);

      webView.resetLoadCounters();

      webView.url = data.url;
      wait(1000);
      compare(webView.loadsStartedCount, 0);
      compare(webView.loadsFailedCount, 0);
      compare(webView.loadsSucceededCount, 0);
      compare(webView.url, url);
    }
  }
}
