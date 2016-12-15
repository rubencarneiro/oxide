import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView

  property bool htmlInputElementFocus: true

  focus: true

  messageHandlers: [
    ScriptMessageHandler {
      msgId: "FOCUS-STATE"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        webView.htmlInputElementFocus = msg.payload;
      }
    }
  ]

  SignalSpy {
    id: spy
    target: webView
    signalName: "activeFocusChanged"
  }

  TestCase {
    id: testcase;
    name: "webview_focus"
    when: windowShown

    function initTestCase() {
      verify(webView.activeFocus);
    }

    function init() {
      spy.clear();

      webView.url = "https://testsuite/tst_focus.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
      "document.addEventListener(\"oxidefocusstate\", function(event) {
        oxide.sendMessage(\"FOCUS-STATE\", event.detail.status);
      });", true);

      verify(webView.activeFocus);
      verify(webView.htmlInputElementFocus);
    }

    function test_toggle_focus_onclicked() {
      var rect = webView.getTestApi().getBoundingClientRectForSelector("#input");

      // clicking outside input field should unfocus the input element.
      var x1 = webView.width / 2;
      var y1 = (rect.y + rect.height + webView.height) / 2;
      mouseClick(webView, x1, y1, Qt.LeftButton);
      verify(webView.activeFocus);
      compare(spy.count, 0);
      tryCompare(webView, "htmlInputElementFocus", false);

      // clicking inside input field restore the focus on input element.
      mouseClick(webView, rect.x + rect.width / 2, rect.y + rect.height / 2, Qt.LeftButton);
      verify(webView.activeFocus);
      compare(spy.count, 0);
      tryCompare(webView, "htmlInputElementFocus", true);
    }

    function test_toggle_focus() {
      webView.focus = false;
      tryCompare(webView, "activeFocus", false);
      tryCompare(webView, "htmlInputElementFocus", false);

      webView.focus = true;
      tryCompare(webView, "activeFocus", true);
      tryCompare(webView, "htmlInputElementFocus", true);
    }
  }
}
