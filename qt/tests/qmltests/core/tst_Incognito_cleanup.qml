import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 1.0

Item {
  id: top

  TestWebContext {
    id: c
  }

  Component {
    id: webViewFactory
    TestWebView {
      incognito: true
      context: c
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

      var obs = OxideTestingUtils.createDestructionObserver(webView);
      webView.destroy();
      webView.waitFor(function() { return obs.destroyed; });

      webView = webViewFactory.createObject(top, {});
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      compare(webView.getTestApi().evaluateCode("document.cookie", false), "");
    }
  }
}
