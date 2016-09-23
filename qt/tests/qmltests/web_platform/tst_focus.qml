import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

Item {
  id: root
  TestWebView {
    id: webView

    property bool htmlInputElementFocus: true;
    focus: true;
    messageHandlers: [
      ScriptMessageHandler {
        msgId: "FOCUS-STATE"
        contexts: [ "oxide://testutils/" ]
        callback: function(msg) {
          webView.htmlInputElementFocus = msg.payload;
        }
      }
    ]
  }

  TestCase {
    name: "webview_focus"
    when: windowShown

    function init() {
      webView.htmlInputElementFocus = true;
    }

    function cleanupTestCase() {

    }

    function test_initial_focus() {
      compare(webView.activeFocus, true, "webView.activeFocus == true");
    }

    function test_toggle_focus() {
      webView.url = "https://testsuite/tst_focus.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.activeFocus, true, "webView.activeFocus == true");
      compare(webView.htmlInputElementFocus, true, "webview.htmlInputElementFocus == true");

      webView.getTestApi().evaluateCode(
      "document.addEventListener(\"oxidefocusstate\", function(event) {
        oxide.sendMessage(\"FOCUS-STATE\", event.detail.status);
      });", true);

      webView.focus = false;
      TestUtils.waitFor(function() { return webView.activeFocus == false; });
      compare(webView.activeFocus, false, "webView.activeFocus == false");
      TestUtils.waitFor(function() { return webView.htmlInputElementFocus == false; });
      compare(webView.htmlInputElementFocus, false, "webview.htmlInputElementFocus == false");

      webView.focus = true;
      TestUtils.waitFor(function() { return webView.activeFocus == false; });
      compare(webView.activeFocus, true, "webView.activeFocus == true");
      TestUtils.waitFor(function() { return webView.htmlInputElementFocus == true; });
      compare(webView.htmlInputElementFocus, true, "webview.htmlInputElementFocus == true");
    }
  }
}
