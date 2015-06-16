import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.4
import com.canonical.Oxide.Testing 1.0

Item {
  id: root
  width: 200
  height: 200

  property var webView

  TestWebContext {
    id: c
  }

  Component {
    id: webViewComponent

    TestWebView {
      context: c

      focus: true
      anchors.fill: parent

      function waitForStateUpdate(state) {
        return waitFor(function() { return (currentState !== state); });
      }
    }
  }

  function get_restore_types() {
    return [
      {restoreType: WebView.RestoreCurrentSession},
      {restoreType: WebView.RestoreLastSessionExitedCleanly},
      {restoreType: WebView.RestoreLastSessionCrashed}
    ];
  }

  TestCase {
    name: "WebView_save_restore_state"
    when: windowShown

    function init() {
      webView = webViewComponent.createObject(root);
    }

    function cleanup() {
      webView.destroy()
    }

    function test_WebView_save_and_restore_current_page_data() {
      return get_restore_types();
    }

    function test_WebView_save_and_restore_current_page(data) {
      webView.url = "http://testsuite/tst_WebView_navigation1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var state = webView.currentState;
      verify(state.length > 0);

      var restored = webViewComponent.createObject(
          root, {"restoreType": data.restoreType, "restoreState": state});
      verify(restored !== null);
      tryCompare(restored, "url", webView.url);
      verify(restored.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      restored.destroy();
    }

    function test_WebView_save_and_restore_navigation_history_data() {
      return get_restore_types();
    }

    function test_WebView_save_and_restore_navigation_history(data) {
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

      var state = webView.currentState;
      verify(state.length > 0);

      var restored = webViewComponent.createObject(
          root, {"restoreType": data.restoreType, "restoreState": state});
      verify(restored !== null);
      tryCompare(restored, "url", webView.url);
      verify(restored.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      verify(restored.canGoBack);
      verify(restored.canGoForward);
      compare(restored.navigationHistory.currentIndex, 1);
      restored.destroy();
    }

    function test_WebView_save_and_restore_scroll_offset_data() {
      return get_restore_types();
    }

    function test_WebView_save_and_restore_scroll_offset(data) {
      webView.url = "http://testsuite/tst_WebView_flickableLikeAPI.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      webView.getTestApi().evaluateCode("document.body.scrollLeft = 300");
      webView.getTestApi().evaluateCode("document.body.scrollTop = 700");
      verify(webView.waitForStateUpdate(webView.currentState));

      var state = webView.currentState;
      verify(state.length > 0);

      var restored = webViewComponent.createObject(
          root, {"restoreType": data.restoreType, "restoreState": state});
      verify(restored !== null);
      tryCompare(restored, "url", webView.url);
      verify(restored.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      verify(restored.waitFor(function() {
          return parseFloat(restored.getTestApi().evaluateCode(
              "document.body.scrollLeft")) == 300; }));
      verify(restored.waitFor(function() {
          return parseFloat(restored.getTestApi().evaluateCode(
              "document.body.scrollTop")) == 700; }));
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
      verify(webView.waitForStateUpdate(webView.currentState));

      var state = webView.currentState;
      verify(state.length > 0);

      var restored = webViewComponent.createObject(
          root,
          {"restoreType": WebView.RestoreCurrentSession, "restoreState": state});
      verify(restored !== null);
      tryCompare(restored, "url", webView.url);
      verify(restored.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      verify(restored.waitFor(function() {
          return restored.getTestApi().evaluateCode(
              "document.querySelector('#textInput').value") == "Te$t"; }));
      restored.destroy();
    }

    function test_WebView_save_and_restore_error_page_data() {
      return get_restore_types();
    }

    // Test that restoring the state of a navigation that had triggered an error
    // triggers the error again (see https://launchpad.net/bugs/1423531).
    function test_WebView_save_and_restore_error_page(data) {
      var url = "http://invalid/";
      webView.url = url;
      verify(webView.waitForLoadCommitted(),
             "Timed out waiting for error page");

      var state = webView.currentState;
      verify(state.length > 0);

      var restored = webViewComponent.createObject(
          root, {"restoreType": data.restoreType, "restoreState": state});
      verify(restored !== null);

      var expectedLoadEvents = [
        { type: LoadEvent.TypeStarted, isError: false },
        { type: LoadEvent.TypeFailed, isError: false },
        { type: LoadEvent.TypeCommitted, isError: true }
      ];
      function _loadEvent(event) {
        var expected = expectedLoadEvents[0];
        compare(event.type, expected.type);
        compare(event.url, url);
        compare(event.isError, expected.isError);
        expectedLoadEvents.shift();
      }
      restored.loadEvent.connect(_loadEvent);

      tryCompare(restored, "url", url);
      restored.waitFor(function() {
          return (expectedLoadEvents.length === 0);
      });
      restored.destroy();
    }
  }
}
