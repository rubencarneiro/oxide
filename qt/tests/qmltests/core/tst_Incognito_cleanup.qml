import QtQuick 2.0
import QtTest 1.0
import Oxide.testsupport 1.0

Item {
  id: top

  Component {
    id: webViewFactory
    TestWebView {
      incognito: true
    }
  }

  TestCase {
    id: test
    name: "Incognito_cleanup"
    when: windowShown

    // Verify that the OTR browsing context is destroyed as soon as the
    // last incognito webview using it is destroyed
    function test_Incognito_cleanup() {
      var webView = webViewFactory.createObject(top, {});
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("document.cookie = \"foo=bar\"", false);
      compare(webView.getTestApi().evaluateCode("document.cookie", false), "foo=bar");

      var webViewHelper = TestSupport.createQObjectTestHelper(webView);
      webView.destroy();
      TestUtils.waitFor(function() { return webViewHelper.destroyed; });

      webView = webViewFactory.createObject(top, {});
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      compare(webView.getTestApi().evaluateCode("document.cookie", false), "");
    }
  }
}
