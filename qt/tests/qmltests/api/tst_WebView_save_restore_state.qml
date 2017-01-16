import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.4
import Oxide.testsupport 1.0

Item {
  id: root

  property var webView

  Component {
    id: webViewComponent

    TestWebView {
      focus: true
      anchors.fill: parent

      function waitForStateUpdate(state) {
        return TestUtils.waitFor(function() { return (currentState !== state); });
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
      verify(TestUtils.waitFor(function() {
          return parseFloat(restored.getTestApi().evaluateCode(
              "document.body.scrollLeft")) == 300; }));
      verify(TestUtils.waitFor(function() {
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
      TestUtils.waitFor(function() {
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
      verify(TestUtils.waitFor(function() {
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
      TestUtils.waitFor(function() {
          return (expectedLoadEvents.length === 0);
      });
      restored.destroy();
    }

    function test_WebView_restore_from_old_state_data() {
      // All serialized state have version 1 (oxide 1.19.6 and older) and they
      // contain three navigation entries (current index is 2).
      return [
        // State serialized with oxide 1.18 or older and restored with oxide
        // 1.19 or newer (https://launchpad.net/bugs/1649861).
        {state:
          "sAQAAAUAAABveGlkZQAAAAEAAAADAAAAAAAAADUAAABodHRwOi8vdGVzdHN1aXRlL3Rz\
           dF9XZWJWaWV3X3NhdmVfcmVzdG9yZV9lbnRyeTEuaHRtbAAAAAAAAADcAAAA2AAAABcA\
           AAAAAAAAagAAAGgAdAB0AHAAOgAvAC8AdABlAHMAdABzAHUAaQB0AGUALwB0AHMAdABf\
           AFcAZQBiAFYAaQBlAHcAXwBzAGEAdgBlAF8AcgBlAHMAdABvAHIAZQBfAGUAbgB0AHIA\
           eQAxAC4AaAB0AG0AbAAAAP////8AAAAAAAAAAP////8AAAAACAAAAAAAAAAAAPA/bZlL\
           pUxFBQBumUulTEUFAAIAAAAIAAAAAAAAAAAAAAAIAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
           AAD/////AAAAAAEAAAgAAAAAAAAAAAEAAAA1AAAAaHR0cDovL3Rlc3RzdWl0ZS90c3Rf\
           V2ViVmlld19zYXZlX3Jlc3RvcmVfZW50cnkxLmh0bWwAAAAAAAAANwPS7eKjLgAAAAAA\
           yAAAAAEAAAABAAAANQAAAGh0dHA6Ly90ZXN0c3VpdGUvdHN0X1dlYlZpZXdfc2F2ZV9y\
           ZXN0b3JlX2VudHJ5Mi5odG1sAAAAAAAAANwAAADYAAAAFwAAAAAAAABqAAAAaAB0AHQA\
           cAA6AC8ALwB0AGUAcwB0AHMAdQBpAHQAZQAvAHQAcwB0AF8AVwBlAGIAVgBpAGUAdwBf\
           AHMAYQB2AGUAXwByAGUAcwB0AG8AcgBlAF8AZQBuAHQAcgB5ADIALgBoAHQAbQBsAAAA\
           /////wAAAAA\AAAAA/////wAAAAAIAAAAAAAAAAAA8D9vmUulTEUFAHCZS6VMRQUAAgA\
           AAAgAAAAAAAAAAAAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAP////8AAAAAAQAACAA\
           AAAAAAAAAAQAAADUAAABodHRwOi8vdGVzdHN1aXRlL3RzdF9XZWJWaWV3X3NhdmVfcmV\
           zdG9yZV9lbnRyeTIuaHRtbAAAAAAAAAACKdPt4qMuAAAAAADIAAAAAQAAAAIAAAA1AAA\
           AaHR0cDovL3Rlc3RzdWl0ZS90c3RfV2ViVmlld19zYXZlX3Jlc3RvcmVfZW50cnkzLmh\
           0bWwAAAAAAAAA3AAAANgAAAAXAAAAAAAAAGoAAABoAHQAdABwADoALwAvAHQAZQBzAHQ\
           AcwB1AGkAdABlAC8AdABzAHQAXwBXAGUAYgBWAGkAZQB3AF8AcwBhAHYAZQBfAHIAZQB\
           zAHQAbwByAGUAXwBlAG4AdAByAHkAMwAuAGgAdABtAGwAAAD/////AAAAAAAAAAD////\
           /AAAAAAgAAAAAAAAAAAAAAHGZS6VMRQUAcplLpUxFBQACAAAACAAAAAAAAAAAAAAACAA\
           AAAAAAAAAAAAAAAAAAAAAAAAAAAAA/////wAAAAABAAAIAAAAAAAAAAABAAAANQAAAGh\
           0dHA6Ly90ZXN0c3VpdGUvdHN0X1dlYlZpZXdfc2F2ZV9yZXN0b3JlX2VudHJ5My5odG1\
           sAAAAAAAAAB8q1O3ioy4AAAAAAMgAAAABAAAAAgAAAA=="},

        // State serialized with oxide >=1.19.0 and <=1.19.6 and restored with
        // oxide 1.19.7 or newer.
        {state:
          "vAQAAAUAAABveGlkZQAAAAEAAAADAAAAAAAAADUAAABodHRwOi8vdGVzdHN1aXRlL3Rz\
           dF9XZWJWaWV3X3NhdmVfcmVzdG9yZV9lbnRyeTEuaHRtbAAAAAAAAADcAAAA2AAAABcA\
           AAAAAAAAagAAAGgAdAB0AHAAOgAvAC8AdABlAHMAdABzAHUAaQB0AGUALwB0AHMAdABf\
           AFcAZQBiAFYAaQBlAHcAXwBzAGEAdgBlAF8AcgBlAHMAdABvAHIAZQBfAGUAbgB0AHIA\
           eQAxAC4AaAB0AG0AbAAAAP////8AAAAAAAAAAP////8AAAAACAAAAAAAAAAAAPA/xS5g\
           QuhFBQDGLmBC6EUFAAIAAAAIAAAAAAAAAAAAAAAIAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
           AAD/////AAAAAAEAAAgAAAAAAAAAAAEAAAA1AAAAaHR0cDovL3Rlc3RzdWl0ZS90c3Rf\
           V2ViVmlld19zYXZlX3Jlc3RvcmVfZW50cnkxLmh0bWwAAAAAAAAA55Pmin6kLgAAAAAA\
           yAAAAAEAAAAAAAAAAQAAADUAAABodHRwOi8vdGVzdHN1aXRlL3RzdF9XZWJWaWV3X3Nh\
           dmVfcmVzdG9yZV9lbnRyeTIuaHRtbAAAAAAAAADcAAAA2AAAABcAAAAAAAAAagAAAGgA\
           dAB0AHAAOgAvAC8AdABlAHMAdABzAHUAaQB0AGUALwB0AHMAdABfAFcAZQBiAFYAaQBl\
           AHcAXwBzAGEAdgBlAF8AcgBlAHMAdABvAHIAZQBfAGUAbgB0AHIAeQAyAC4AaAB0AG0A\
           bAAAAP////8AAAAAAAAAAP////8AAAAACAAAAAAAAAAAAPA/xy5gQuhFBQDILmBC6EUF\
           AAIAAAAIAAAAAAAAAAAAAAAIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD/////AAAAAAEA\
           AAgAAAAAAAAAAAEAAAA1AAAAaHR0cDovL3Rlc3RzdWl0ZS90c3RfV2ViVmlld19zYXZl\
           X3Jlc3RvcmVfZW50cnkyLmh0bWwAAAAAAAAA9eLnin6kLgAAAAAAyAAAAAEAAAAAAAAA\
           AgAAADUAAABodHRwOi8vdGVzdHN1aXRlL3RzdF9XZWJWaWV3X3NhdmVfcmVzdG9yZV9l\
           bnRyeTMuaHRtbAAAAAAAAADcAAAA2AAAABcAAAAAAAAAagAAAGgAdAB0AHAAOgAvAC8A\
           dABlAHMAdABzAHUAaQB0AGUALwB0AHMAdABfAFcAZQBiAFYAaQBlAHcAXwBzAGEAdgBl\
           AF8AcgBlAHMAdABvAHIAZQBfAGUAbgB0AHIAeQAzAC4AaAB0AG0AbAAAAP////8AAAAA\
           AAAAAP////8AAAAACAAAAAAAAAAAAAAAyS5gQuhFBQDKLmBC6EUFAAIAAAAIAAAAAAAA\
           AAAAAAAIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD/////AAAAAAEAAAgAAAAAAAAAAAEA\
           AAA1AAAAaHR0cDovL3Rlc3RzdWl0ZS90c3RfV2ViVmlld19zYXZlX3Jlc3RvcmVfZW50\
           cnkzLmh0bWwAAAAAAAAA5o/oin6kLgAAAAAAyAAAAAEAAAAAAAAAAgAAAA=="}
      ];
    }

    function test_WebView_restore_from_old_state(data) {
      var restored = webViewComponent.createObject(
          root,
          {"restoreType": WebView.RestoreCurrentSession,
           "restoreState": data.state});
      verify(restored !== null);
      tryCompare(restored, "url",
                 "http://testsuite/tst_WebView_save_restore_entry3.html");
      verify(restored.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compare(restored.navigationHistory.currentItemIndex, 2);
      var items = restored.navigationHistory.items;
      for (var i = 0; i < 3; ++i) {
        compare(items[i].url,
                "http://testsuite/tst_WebView_save_restore_entry%1.html"
                    .arg(i + 1));
      }
      restored.destroy();
    }
  }
}
