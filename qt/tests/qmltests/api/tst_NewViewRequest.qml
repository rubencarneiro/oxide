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

    property string lastRequestUrl: ""
    property rect lastRequestPosition: Qt.rect(0,0,0,0)
    property int lastRequestDisposition: NewViewRequest.DispositionCurrentTab
    property bool lastRequestUserGesture: false

    onNewViewRequested: {
      lastRequestUrl = request.url;
      lastRequestPosition = request.position;
      lastRequestDisposition = request.disposition;
      lastRequestUserGesture = request.userGesture;
    }
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "newViewRequested"
  }

  TestCase {
    id: test
    name: "NewViewRequest"
    when: windowShown

    function init() {
      webView.context.popupBlockerEnabled = true;
      spy.clear()
    }

    function test_NewViewRequest1_from_user_gesture_data() {
      return [
        { selector: "#button1", modifiers: Qt.NoModifier, disposition: NewViewRequest.DispositionNewForegroundTab },
        { selector: "#button1", modifiers: Qt.ShiftModifier, disposition: NewViewRequest.DispositionNewWindow },
        { selector: "#button1", modifiers: Qt.ControlModifier, disposition: NewViewRequest.DispositionNewBackgroundTab },
        { selector: "#button1", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NewViewRequest.DispositionNewForegroundTab },
        { selector: "#button2", modifiers: Qt.NoModifier, disposition: NewViewRequest.DispositionNewPopup },
        { selector: "#button2", modifiers: Qt.ShiftModifier, disposition: NewViewRequest.DispositionNewPopup },
        { selector: "#button2", modifiers: Qt.ControlModifier, disposition: NewViewRequest.DispositionNewBackgroundTab },
        { selector: "#button2", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NewViewRequest.DispositionNewForegroundTab }
      ];
    }

    function test_NewViewRequest1_from_user_gesture(data) {
      webView.url = "http://localhost:8080/tst_NewViewRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, data.modifiers);

      spy.wait();

      compare(webView.lastRequestUrl, "http://localhost:8080/empty.html",
              "Unexpected URL");
      // XXX: This will fail if the request is outside of the screen's availableRect
      //  (including shell chrome). The figures are set to make this unlikely though
      compare(webView.lastRequestPosition.x, 100, "Unexpected position.x");
      compare(webView.lastRequestPosition.y, 100, "Unexpected position.y");
      compare(webView.lastRequestPosition.width, 200, "Unexpected position.width");
      compare(webView.lastRequestPosition.height, 200, "Unexpected position.height");
      compare(webView.lastRequestDisposition, data.disposition, "Unexpected disposition");
      compare(webView.lastRequestUserGesture, true, "Came from user gesture");
    }

    function test_NewViewRequest2_no_user_gesture_data() {
      return test_NewViewRequest1_from_user_gesture_data();
    }

    function test_NewViewRequest2_no_user_gesture(data) {
      webView.context.popupBlockerEnabled = false;

      webView.url = "http://localhost:8080/tst_NewViewRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"var e = document.createEvent(\"HTMLEvents\");
e.initEvent(\"click\", true, false);
document.querySelector(\"" + data.selector + "\").dispatchEvent(e);", true);

      spy.wait();

      compare(webView.lastRequestUrl, "http://localhost:8080/empty.html",
              "Unexpected URL");
      // XXX: This will fail if the request is outside of the screen's availableRect
      //  (including shell chrome). The figures are set to make this unlikely though
      compare(webView.lastRequestPosition.x, 100, "Unexpected position.x");
      compare(webView.lastRequestPosition.y, 100, "Unexpected position.y");
      compare(webView.lastRequestPosition.width, 200, "Unexpected position.width");
      compare(webView.lastRequestPosition.height, 200, "Unexpected position.height");
      // RenderViewImpl::show() coerces this to NewPopup because it didn't originate from
      // a user gesture
      compare(webView.lastRequestDisposition, NewViewRequest.DispositionNewPopup,
              "Unexpected disposition");
      compare(webView.lastRequestUserGesture, false, "Didn't come from user gesture");
    }
  }
}
