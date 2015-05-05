import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.4
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
      webView.clearLoadEventCounters();
    }

    // Verify that window.close() is ignored for non-script-opened windows
    // by default when there is more than one entry in the navigation history
    function test_WebView_closeRequested1_application_opened_default() {
      verify(!webView.preferences.allowScriptsToCloseWindows);

      webView.url = "http://foo.testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("window.close();", false);
      compare(spy.count, 1);

      webView.url = "http://bar.testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());
      webView.url = "http://foo.testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("window.close();", false);
      compare(spy.count, 1);
    }

    // Verify that window.close() works for non-script-opened windows
    // when the pref is configured to allow it
    function test_WebView_closeRequested2_application_opened_allowed() {
      webView.url = "http://bar.testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://foo.testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("window.close();", false);
      compare(spy.count, 0);

      webView.preferences.allowScriptsToCloseWindows = true;

      webView.getTestApi().evaluateCode("window.close();", false);
      compare(spy.count, 1);

      webView.preferences.allowScriptsToCloseWindows = false;
    }

    function test_WebView_closeRequested3_script_opened() {
      webView.url = "http://bar.testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      verify(!webView.preferences.allowScriptsToCloseWindows);
      webView.context.popupBlockerEnabled = false;

      webView.getTestApi().evaluateCode("window.open(\"empty.html\");", true);
      webView.waitFor(function() { return column.created != null; });

      var created = column.created;
      spy.target = created;

      verify(!created.preferences.allowScriptsToCloseWindows);

      created.url = "http://testsuite/empty.html";
      verify(created.waitForLoadSucceeded());

      created.getTestApi().evaluateCode("window.close();", false);
      compare(spy.count, 1);

      // Don't navigate |created| after this. We want to leave it in its "closed"
      // state to make sure the test process shuts down correctly
    }
  }
}
