import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

Column {
  id: column
  focus: true

  TestWebView {
    id: webView
    width: 200
    height: 200

    property rect lastRequestPosition: Qt.rect(0,0,0,0)
    property int lastRequestDisposition: NewViewRequest.DispositionCurrentTab

    Component {
      id: webViewFactory
      WebView {}
    }

    onNewViewRequested: {
      lastRequestPosition = request.position;
      lastRequestDisposition = request.disposition;
      webViewFactory.createObject(webView, { request: request });
    }
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "newViewRequested"
  }

  SignalSpy {
    id: navSpy
    target: webView
    signalName: "navigationRequested"
  }

  TestCase {
    id: test
    name: "NewViewRequest"
    when: windowShown

    function init() {
      webView.context.popupBlockerEnabled = true;
      spy.clear();
      navSpy.clear();
    }

    function test_NewViewRequest1_from_user_gesture_data() {
      return [
        { selector: "#button1", modifiers: Qt.NoModifier, disposition: NewViewRequest.DispositionNewForegroundTab, style: "window-open" },
        { selector: "#button1", modifiers: Qt.ShiftModifier, disposition: NewViewRequest.DispositionNewWindow, style: "window-open" },
        { selector: "#button1", modifiers: Qt.ControlModifier, disposition: NewViewRequest.DispositionNewBackgroundTab, style: "window-open" },
        { selector: "#button1", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NewViewRequest.DispositionNewForegroundTab, style: "window-open" },
        { selector: "#button2", modifiers: Qt.NoModifier, disposition: NewViewRequest.DispositionNewPopup, style: "window-open" },
        { selector: "#button2", modifiers: Qt.ShiftModifier, disposition: NewViewRequest.DispositionNewPopup, style: "window-open" },
        { selector: "#button2", modifiers: Qt.ControlModifier, disposition: NewViewRequest.DispositionNewBackgroundTab, style: "window-open" },
        { selector: "#button2", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NewViewRequest.DispositionNewForegroundTab, style: "window-open" },
        { selector: "#link1", modifiers: Qt.NoModifier, disposition: NewViewRequest.DispositionNewForegroundTab, style: "link-new-window" },
        { selector: "#link1", modifiers: Qt.ShiftModifier, disposition: NewViewRequest.DispositionNewWindow, style: "link-new-window" },
        { selector: "#link1", modifiers: Qt.ControlModifier, disposition: NewViewRequest.DispositionNewBackgroundTab, style: "link-new-window" },
        { selector: "#link1", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NewViewRequest.DispositionNewForegroundTab, style: "link-new-window" },
        { selector: "#link2", modifiers: Qt.NoModifier, disposition: NewViewRequest.DispositionCurrentTab, style: "link" },
        { selector: "#link2", modifiers: Qt.ShiftModifier, disposition: NewViewRequest.DispositionNewWindow, style: "link" },
        { selector: "#link2", modifiers: Qt.ControlModifier, disposition: NewViewRequest.DispositionNewBackgroundTab, style: "link" },
        { selector: "#link2", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NewViewRequest.DispositionNewForegroundTab, style: "link" }
      ];
    }

    function test_NewViewRequest1_from_user_gesture(data) {
      webView.url = "http://testsuite/tst_NewViewRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, data.modifiers);

      if (data.disposition == NewViewRequest.DispositionCurrentTab) {
        verify(webView.waitForLoadSucceeded());
        compare(spy.count, 0);
      } else {
        spy.wait();
        compare(spy.count, 1);

        if (data.style != "link-new-window") {
          // XXX: This will fail if the request is outside of the screen's availableRect
          //  (including shell chrome). The figures are set to make this unlikely though
          if (data.style == "window-open") {
            compare(webView.lastRequestPosition.x, 100, "Unexpected position.x");
            compare(webView.lastRequestPosition.y, 100, "Unexpected position.y");
          }
          compare(webView.lastRequestPosition.width, data.style == "window-open" ? 200 : webView.width, "Unexpected position.width");
          compare(webView.lastRequestPosition.height, data.style == "window-open" ? 200 : webView.height, "Unexpected position.height");
        }
        compare(webView.lastRequestDisposition, data.disposition, "Unexpected disposition");
      }

      compare(navSpy.count, 1);
    }

    function test_NewViewRequest2_no_user_gesture_data() {
      return test_NewViewRequest1_from_user_gesture_data();
    }

    function test_NewViewRequest2_no_user_gesture(data) {
      webView.context.popupBlockerEnabled = false;

      if (data.style == "link") {
        return;
      }

      webView.url = "http://testsuite/tst_NewViewRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"var e = document.createEvent(\"HTMLEvents\");
e.initEvent(\"click\", true, false);
document.querySelector(\"" + data.selector + "\").dispatchEvent(e);", true);

      spy.wait();

      compare(spy.count, 1);
      compare(navSpy.count, 1);

      if (data.style != "link-new-window") {
        // XXX: This will fail if the request is outside of the screen's availableRect
        //  (including shell chrome). The figures are set to make this unlikely though
        compare(webView.lastRequestPosition.x, 100, "Unexpected position.x");
        compare(webView.lastRequestPosition.y, 100, "Unexpected position.y");
        compare(webView.lastRequestPosition.width, 200, "Unexpected position.width");
        compare(webView.lastRequestPosition.height, 200, "Unexpected position.height");
      }
      compare(webView.lastRequestDisposition, NewViewRequest.DispositionNewPopup,
              "Unexpected disposition");
    }
  }
}
