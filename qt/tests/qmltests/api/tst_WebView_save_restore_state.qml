import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestCase {
  name: "WebView_save_restore_state"
  when: windowShown
  width: 200
  height: 200

  property var webView

  Component {
    id: webViewComponent

    TestWebView {
      focus: true
      anchors.fill: parent
    }
  }

  function init() {
    webView = webViewComponent.createObject(this);
  }

  function cleanup() {
    webView.destroy()
  }

  function test_WebView_save_and_restore_current_page() {
    webView.url = "http://testsuite/tst_WebView_navigation1.html";
    verify(webView.waitForLoadSucceeded(),
           "Timed out waiting for successful load");

    var state = webView.currentState();
    verify(state.length > 0);

    var restored = webViewComponent.createObject(webView, {"state": state});
    verify(restored !== null);
    tryCompare(restored, "url", webView.url);
    verify(restored.waitForLoadSucceeded(),
           "Timed out waiting for successful load");
    restored.destroy();
  }

  function test_WebView_save_and_restore_navigation_history() {
    webView.url = "http://testsuite/tst_WebView_navigation1.html";
    verify(webView.waitForLoadSucceeded(),
           "Timed out waiting for successful load");
    webView.url = "http://testsuite/tst_WebView_navigation2.html";
    verify(webView.waitForLoadSucceeded(),
           "Timed out waiting for successful load");
    webView.url = "http://testsuite/tst_WebView_navigation3.html";
    verify(webView.waitForLoadSucceeded(),
           "Timed out waiting for successful load");
    webView.goBack();
    verify(webView.waitForLoadSucceeded(),
           "Timed out waiting for successful load");

    var state = webView.currentState();
    verify(state.length > 0);

    var restored = webViewComponent.createObject(webView, {"state": state});
    verify(restored !== null);
    tryCompare(restored, "url", webView.url);
    verify(restored.waitForLoadSucceeded(),
           "Timed out waiting for successful load");
    verify(restored.canGoBack);
    verify(restored.canGoForward);
    compare(restored.navigationHistory.currentIndex, 1);
    restored.destroy();
  }

  function test_WebView_save_and_restore_scroll_offset() {
    webView.url = "http://testsuite/tst_WebView_flickableLikeAPI.html";
    verify(webView.waitForLoadSucceeded(),
           "Timed out waiting for successful load");
    webView.getTestApi().evaluateCode("document.body.scrollLeft = 300");
    webView.getTestApi().evaluateCode("document.body.scrollTop = 700");

    var state = webView.currentState();
    verify(state.length > 0);

    var restored = webViewComponent.createObject(webView, {"state": state});
    verify(restored !== null);
    tryCompare(restored, "url", webView.url);
    verify(restored.waitForLoadSucceeded(),
           "Timed out waiting for successful load");
    restored.waitFor(function() {
        return parseFloat(restored.getTestApi().evaluateCode(
            "document.body.scrollLeft")) == 300; });
    restored.waitFor(function() {
        return parseFloat(restored.getTestApi().evaluateCode(
            "document.body.scrollTop")) == 700; });
    restored.destroy();
  }

  function test_WebView_save_and_restore_form_data_input() {
    webView.url = "http://testsuite/tst_WebView_save_restore_form_data.html";
    verify(webView.waitForLoadSucceeded(),
           "Timed out waiting for successful load");
    keyClick("T");
    keyClick("e");
    keyClick("$");
    keyClick("t");
    webView.waitFor(function() {
        return webView.getTestApi().evaluateCode(
            "document.querySelector('#textInput').value") == "Te$t"; });

    var state = webView.currentState();
    verify(state.length > 0);

    var restored = webViewComponent.createObject(webView, {"state": state});
    verify(restored !== null);
    tryCompare(restored, "url", webView.url);
    verify(restored.waitForLoadSucceeded(),
           "Timed out waiting for successful load");
    restored.waitFor(function() {
        return restored.getTestApi().evaluateCode(
            "document.querySelector('#textInput').value") == "Te$t"; });
    restored.destroy();
  }
}
