import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

Column {
  id: column
  focus: true

  TestWebView {
    id: webView
    width: 200
    height: 200

    property string lastRequestUrl: ""
    property int lastRequestDisposition: NewViewRequest.DispositionCurrentTab
    property bool lastRequestUserGesture: false

    onNewViewRequested: {
      lastRequestUrl = request.url;
      lastRequestDisposition = request.disposition;
      lastRequestUserGesture = request.userGesture;
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
        { modifiers: Qt.NoModifier, disposition: NewViewRequest.DispositionNewForegroundTab },
        { modifiers: Qt.ShiftModifier, disposition: NewViewRequest.DispositionNewWindow },
        { modifiers: Qt.ControlModifier, disposition: NewViewRequest.DispositionNewBackgroundTab },
        { modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NewViewRequest.DispositionNewForegroundTab }
      ];
    }

    // Verify that anchor elements with target="_blank" generate
    // onNewViewRequested signals, and that they don't generate
    // onNavigationRequested signals (these result in new windows created via
    // createWindowForRequest() in blink, bypassing the normal
    // decidePolicyForNavigation path - see WebCore::FrameLoader::load())
    function test_NewViewRequest1_from_user_gesture(data) {
      webView.url = "http://localhost:8080/tst_NewViewRequest_anchor.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView.getTestApi().getBoundingClientRectForSelector("#anchor");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, data.modifiers);

      spy.wait();

      compare(navSpy.count, 0, "Shouldn't get onNavigationRequested for anchor elements with target=_blank");

      compare(webView.lastRequestUrl, "http://localhost:8080/empty.html",
              "Unexpected URL");
      compare(webView.lastRequestDisposition, data.disposition, "Unexpected disposition");
      compare(webView.lastRequestUserGesture, true, "Came from user gesture");
    }

    function test_NewViewRequest2_no_user_gesture_data() {
      return test_NewViewRequest1_from_user_gesture_data();
    }

    function test_NewViewRequest2_no_user_gesture(data) {
      webView.context.popupBlockerEnabled = false;

      webView.url = "http://localhost:8080/tst_NewViewRequest_anchor.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"var e = document.createEvent(\"HTMLEvents\");
e.initEvent(\"click\", true, false);
document.querySelector(\"#anchor\").dispatchEvent(e);", true);

      spy.wait();

      compare(navSpy.count, 0, "Shouldn't get onNavigationRequested for anchor elements with target=_blank");

      compare(webView.lastRequestUrl, "http://localhost:8080/empty.html",
              "Unexpected URL");
      // RenderViewImpl::show() coerces this to NewPopup because it didn't originate from
      // a user gesture
      compare(webView.lastRequestDisposition, NewViewRequest.DispositionNewPopup,
              "Unexpected disposition");
      compare(webView.lastRequestUserGesture, false, "Didn't come from user gesture");
    }
  }
}
