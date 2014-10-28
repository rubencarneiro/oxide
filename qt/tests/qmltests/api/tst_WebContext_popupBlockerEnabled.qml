import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

Column {
  id: column
  focus: true

  Component {
    id: webViewFactory
    WebView {}
  }

  TestWebView {
    id: webView
    width: 200
    height: 200

    onNewViewRequested: {
      webViewFactory.createObject(column, { request: request, width: 200, height: 200 });
    }
  }

  TestCase {
    id: test
    name: "WebContext_popupBlockerEnabled"
    when: windowShown

    function test_WebContext_popupBlockerEnabled1_default() {
      compare(webView.context.popupBlockerEnabled, true,
              "Popup blocker should be enabled by default");
    }

    function test_WebContext_popupBlockerEnabled2_disabled() {
      webView.context.popupBlockerEnabled = false;

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      verify(webView.getTestApi().evaluateCode(
          "return window.open(\"empty.html\") != null;", true),
          "Should be able to open window without popup blocker");
    }

    function test_WebContext_popupBlockerEnabled3_enabled() {
      webView.context.popupBlockerEnabled = true;

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      verify(webView.getTestApi().evaluateCode(
          "return window.open(\"empty.html\") == null;", true),
          "Shouldn't be able to open window with popup blocker");
    }
  }
}
