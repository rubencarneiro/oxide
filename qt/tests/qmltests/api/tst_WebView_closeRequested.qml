import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

Column {
  id: column

  Component {
    id: webViewFactory
    TestWebView {
      width: 200
      height: 200
      context: WebContext {}
    }
  }

  property var created: null

  TestWebView {
    id: webView
    width: 200
    height: 200

    onNewViewRequested: {
      created = webViewFactory.createObject(column, { request: request });
    }
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "closeRequested"
  }

  TestCase {
    id: test
    name: "WebView_closeRequested"
    when: windowShown

    function init() {
      spy.clear();
    }

    // Verify that window.close() is ignored for non-script-opened windows
    // by default when there is more than one entry in the navigation history
    function test_WebView_closeRequested1_application_opened_default() {
      verify(!webView.preferences.allowScriptsToCloseWindows);

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("window.close();", false);
      compare(spy.count, 1);

      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded());
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("window.close();", false);
      compare(spy.count, 1);
    }

    // Verify that window.close() works for non-script-opened windows
    // when the pref is configured to allow it
    function test_WebView_closeRequested2_application_opened_allowed() {
      webView.preferences.allowScriptsToCloseWindows = true;

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("window.close();", false);
      compare(spy.count, 1);

      webView.preferences.allowScriptsToCloseWindows = false;
    }

    function test_WebView_closeRequested3_script_opened() {
      verify(!webView.preferences.allowScriptsToCloseWindows);
      webView.context.popupBlockerEnabled = false;

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("window.open(\"empty.html\");", true);
      webView.waitFor(function() { return column.created != null; });

      var created = column.created;
      spy.target = created;

      created.getTestApi().evaluateCode("window.close();", false);
      compare(spy.count, 1);
    }
  }
}
