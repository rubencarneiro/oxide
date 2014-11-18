import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    signalName: "urlChanged"
  }

  Component.onCompleted: {
    spy.target = webView.rootFrame;
  }

  TestCase {
    id: test
    name: "WebFrame_url"
    when: windowShown

    function initTestCase() {
      OxideTestingUtils.setUrlHandler("foo", false);
    }

    function cleanupTestCase() {
      OxideTestingUtils.unsetUrlHandler("foo");
    }

    function test_WebFrame_url1() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(spy.count, 1, "Incorrect number of urlChanged signals");
      compare(webView.rootFrame.url, webView.url,
              "url should match webview url");

      webView.url = "foo://bar.com";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(spy.count, 2, "Expected another urlChanged signal");
      compare(webView.rootFrame.url, webView.url,
              "url should match webview url");
    }
  }
}
