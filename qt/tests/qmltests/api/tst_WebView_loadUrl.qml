import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property var lastUrl: ""

  onUrlChanged: {
    if (url == lastUrl) {
      fail("Got a urlChanged() signal when the url didn't change");
    }
    lastUrl = url;
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "urlChanged"
  }

  TestCase {
    id: test
    name: "WebView_loadUrl"
    when: windowShown

    function init() {
      webView.clearLoadEventCounters();
      spy.clear();
    }

    function test_WebView_loadUrl1_data() {
      return [
        { url: "http://localhost:8080/empty.html", succeeded: 1, failed: 0 },
        { url: Qt.resolvedUrl("./empty.html"), succeeded: 1, failed: 0 },
        { url: "about:blank", succeeded: 1, failed: 0 },
        { url: "foo://bar.com", succeeded: 1, failed: 1, documentURI: "data:text/html,chromewebdata" }
      ];
    }

    function test_WebView_loadUrl1(data) {
      webView.url = data.url;
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      compare(spy.count, 1, "Got an unexpected number of url changes");

      compare(webView.loadsSucceededCount, data.succeeded,
              "Got an unexpected number of successful loads");
      compare(webView.loadsFailedCount, data.failed,
              "Got an unexpected number of failed loads");
      compare(webView.loadsStartedCount, data.succeeded + data.failed,
              "Got an unexpected number of started loads");
      compare(webView.url, data.url,
              "WebView.url is incorrect");

      if (!("documentURI" in data)) {
        data.documentURI = data.url;
      }

      compare(webView.getTestApi().documentURI, data.documentURI,
              "document.documentURI is incorrect");
    }

    function test_WebView_loadUrl2_ignoreInvalid_data() {
      return [
        { url: "" }
      ];
    }

    function test_WebView_loadUrl2_ignoreInvalid(data) {
      var url = "http://localhost:8080/empty.html";

      webView.url = url;
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      compare(spy.count, 1, "Got an unexpected number of url changes");

      compare(webView.loadsStartedCount, 1,
              "Got an unexpected number of started loads");
      compare(webView.loadsSucceededCount, 1,
              "Got an unexpected number of successful loads");
      compare(webView.loadsFailedCount, 0,
              "Got an unexpected number of failed loads");
      compare(webView.url, url, "WebView.url is incorrect");

      webView.clearLoadEventCounters();
      spy.clear();

      webView.url = data.url;
      wait(1000);

      compare(spy.count, 0, "There should have been no url changes");

      compare(webView.loadsStartedCount, 0,
              "There should have been no started loads");
      compare(webView.loadsFailedCount, 0,
              "There should have been no failed loads");
      compare(webView.loadsSucceededCount, 0,
              "There should have been no successful loads");
      compare(webView.url, url,
              "WebView.url should match the original url");
    }
  }

  Component.onCompleted: {
    lastUrl = webView.url;
  }
}
