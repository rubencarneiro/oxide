import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "urlChanged"
  }

  Component {
    id: webViewComponent
    TestWebView {}
  }

  TestCase {
    id: test
    name: "WebView_url"
    when: windowShown

    function init() {
      webView.clearLoadEventCounters();
      spy.clear();
    }

    // Verify that setting WebView.url during construction works
    function test_WebView_url1_construction() {
      var view = webViewComponent.createObject(null, { url: "http://foo.testsuite/empty.html" });
      compare(view.url, "http://foo.testsuite/empty.html");
      verify(view.waitForLoadSucceeded());
      compare(view.getTestApi().documentURI, "http://foo.testsuite/empty.html");
    }

    function test_WebView_url2_browser_initiated_data() {
      return [
        { url: "http://testsuite/empty.html", count: 1 },
        { url: Qt.resolvedUrl("./empty.html"), count: 1 },
        { url: "about:blank", count: 1 },
        { url: "http://invalid/", count: 2, documentURI: "data:text/html,chromewebdata", type: "fail" },
        { url: "http://testsuite/tst_WebView_url_redirect.py", count: 1, finalUrl: "http://testsuite/empty.html" },
        { url: "foo://bar.com", count: 1, finalUrl: "http://testsuite/empty.html", type: "stop" },
      ];
    }

    function test_WebView_url2_browser_initiated(data) {
      var origUrl = webView.url;

      webView.url = data.url;
      compare(spy.count, origUrl != data.url ? 1 : 0);
      compare(webView.url, data.url, "Unexpected URL");

      spy.clear();

      if (data.type == "fail") {
        verify(webView.waitForLoadCommitted(),
               "Timed out waiting for failed load");
      } else if (data.type == "stop") {
        verify(webView.waitForLoadStopped(),
               "Timed out waiting for cancelled load");
      } else {
        verify(webView.waitForLoadSucceeded(),
               "Timed out waiting for successful load");
      }

      if (!("finalUrl" in data)) {
        data.finalUrl = data.url;
      }

      compare(spy.count, data.count, "Got an unexpected number of url changes");
      compare(webView.url, data.finalUrl, "Unexpected URL");

      if (!("documentURI" in data)) {
        data.documentURI = data.finalUrl;
      }

      compare(webView.getTestApi().documentURI, data.documentURI,
              "document.documentURI is incorrect");
    }

    function test_WebView_url3_ignore_invalid_data() {
      return [
        { url: "" }
      ];
    }

    function test_WebView_url3_ignore_invalid(data) {
      var url = "http://testsuite/empty.html";

      webView.url = url;
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      compare(webView.url, url, "WebView.url is incorrect");

      spy.clear();
      webView.clearLoadEventCounters();

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

    // Verify that WebView.url does not indicate the pending URL for
    // content initiated navigations, to prevent URL spoofing
    function test_WebView_url4_content_initiated() {
      var origUrl = "http://testsuite/empty.html";
      var newUrl = "http://foo.testsuite/empty.html";

      webView.url = origUrl;
      verify(webView.waitForLoadSucceeded());
      compare(webView.url, origUrl);

      spy.clear();

      function _onLoadEvent(event) {
        verify(event.type == LoadEvent.TypeStarted ||
               event.type == LoadEvent.TypeCommitted ||
               event.type == LoadEvent.TypeSucceeded);

        compare(spy.count, event.type == LoadEvent.TypeStarted ? 1 : 2);
        compare(webView.url, event.type == LoadEvent.TypeStarted ? origUrl : newUrl);
      }

      webView.loadEvent.connect(_onLoadEvent);

      webView.getTestApi().evaluateCode("window.location = \"" + newUrl + "\"", false);
      verify(webView.waitForLoadSucceeded());

      compare(webView.url, newUrl);

      webView.loadEvent.disconnect(_onLoadEvent);
    }
  }
}
