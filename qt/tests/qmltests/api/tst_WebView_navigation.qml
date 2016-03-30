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
    signalName: "navigationHistoryChanged"
  }

  TestCase {
    id: test
    name: "WebView_navigation"
    when: windowShown

    function init() {
      spy.clear();
    }

    readonly property var initData: [
      { url: "http://testsuite/tst_WebView_navigation1.html", index: 0 },
      { url: "http://testsuite/tst_WebView_navigation2.html", index: 1 },
      { url: "http://testsuite/tst_WebView_navigation3.html", index: 2 },
      { url: "http://testsuite/tst_WebView_navigation1.html", index: 3 }
    ]

    function test_WebView_navigation1_init_data() {
      return initData;
    }

    function test_WebView_navigation1_init(data) {
      if (data.index == 0) {
        verify(!webView.canGoBack, "Should be nothing to go back to");
        verify(!webView.canGoForward, "Should be nothing to go forward to");
      }

      webView.url = data.url;
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for load to finish");

      compare(webView.url, data.url,
              "WebView.url is invalid after load");
      compare(webView.getTestApi().documentURI, data.url,
              "document.documentURI is invalid after load");
      verify(!webView.canGoForward, "Should be nothing to go forward to");
      if (data.index > 0) {
        verify(webView.canGoBack, "Should be able to go back");
      } else {
        verify(!webView.canGoBack, "Should not be able to go back");
      }
      verify(spy.count > 0,
             "We should have had a navigationHistoryChanged signal");
    }

    function test_WebView_navigation2_data() {
      return [
        { direction: "back", expected: 2 },
        { direction: "back", expected: 1 },
        { direction: "back", expected: 0 },
        { direction: "forward", expected: 1 },
        { direction: "forward", expected: 2 },
        { direction: "forward", expected: 3 }
      ]
    }

    function test_WebView_navigation2(data) {
      if (data.direction == "back") {
        verify(webView.canGoBack, "Should be able to go back");
        webView.goBack();
      } else if (data.direction == "forward") {
        verify(webView.canGoForward, "Should be able to go forward");
        webView.goForward();
      }

      if (data.expected == 0) {
        verify(!webView.canGoBack, "Shouldn't be able to go back");
        verify(spy.count > 0);
      } else {
        verify(webView.canGoBack, "Should be able to go back");
      }
      if (data.expected == initData.length - 1) {
        verify(!webView.canGoForward, "Shouldn't be able to go forward");
        verify(spy.count > 0);
      } else {
        verify(webView.canGoForward, "Should be able to go forward");
      }

      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      var expectedUrl = initData[data.expected].url;
      compare(webView.url, url,
              "Unexpected WebView.url after page navigation");
      compare(webView.getTestApi().documentURI, url,
              "Unexpected document.documentURI after page navigation");
    }

    function test_WebView_navigation3() {
      verify(webView.canGoBack, "Should be able to go back");
      verify(!webView.canGoForward, "Shouldn't be able to go forward");

      var data = test_WebView_navigation2_data();
      var index = data[data.length - 1].expected;
      compare(webView.url, initData[index].url, "Unexpected WebView.url");

      webView.goBack();
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.url, initData[--index].url, "Unexpected WebView.url");
      verify(webView.canGoBack, "Should be able to go back");
      verify(webView.canGoForward, "Should be able to go forward");

      spy.clear();
      var url = "http://testsuite/tst_WebView_navigation4.html";

      webView.url = url;
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      compare(webView.url, url, "Unexpected WebView.url");
      verify(webView.canGoBack, "Should be able to go back");
      verify(!webView.canGoForward, "Should no longer be able to go forward");
      verify(spy.count > 0, "Should have had a navigationHistoryChanged signal");
    }

    function test_WebView_navigation4() {
      while (webView.canGoBack) {
        webView.goBack();
        verify(webView.waitForLoadSucceeded());
      }

      // Shouldn't crash
      webView.goBack();

      while (webView.canGoForward) {
        webView.goForward();
        verify(webView.waitForLoadSucceeded());
      }

      // Shouldn't crash
      webView.goForward();
    }
  }
}
