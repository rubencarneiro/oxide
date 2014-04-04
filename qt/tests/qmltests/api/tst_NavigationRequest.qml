import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

// FIXME: Test navigations in a subwindow (ie, one opened with window.open() and with
// window.opener pointing to its parent - we should only get onNavigationRequest
// notifications for cross-domain navigations as opposed to all navigations)

TestWebView {
  id: webView
  width: 200
  height: 200

  property string lastRequestUrl: ""
  property int lastRequestDisposition: NavigationRequest.DispositionCurrentTab
  property bool lastRequestUserGesture: false

  property bool shouldReject: false

  onNavigationRequested: {
    if (shouldReject) {
      request.action = NavigationRequest.ActionReject;
      return;
    }

    lastRequestUrl = request.url;
    lastRequestDisposition = request.disposition;
    lastRequestUserGesture = request.userGesture;
  }

  // XXX(chrisccoulson): If we don't do this, then all future calls to
  // onNewViewRequested fail (we get a null request object)
  onNewViewRequested: {
    var r = request;
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "navigationRequested"
  }

  SignalSpy {
    id: newViewSpy
    target: webView
    signalName: "newViewRequested"
  }

  SignalSpy {
    id: frameSpy
    signalName: "urlChanged"
  }

  TestCase {
    id: test
    name: "NavigationRequest"
    when: windowShown

    function init() {
      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      spy.clear();
      newViewSpy.clear();
      frameSpy.clear();
      webView.shouldReject = false;
      webView.context.popupBlockerEnabled = true;
    }

    function test_NavigationRequest1_from_user_gestures_data() {
      return [
        { link: "#link1", url: "http://localhost:8080/empty.html", modifiers: Qt.NoModifier, disposition: NavigationRequest.DispositionCurrentTab, current: true },
        { link: "#link1", url: "http://localhost:8080/empty.html", modifiers: Qt.ShiftModifier, disposition: NavigationRequest.DispositionNewWindow, current: true },
        { link: "#link1", url: "http://localhost:8080/empty.html", modifiers: Qt.ControlModifier, disposition: NavigationRequest.DispositionNewBackgroundTab, current: true },
        { link: "#link1", url: "http://localhost:8080/empty.html", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NavigationRequest.DispositionNewForegroundTab, current: true },
        { link: "#button1", url: "http://localhost:8080/empty.html", modifiers: Qt.NoModifier, disposition: NavigationRequest.DispositionNewForegroundTab, current: false },
        { link: "#button1", url: "http://localhost:8080/empty.html", modifiers: Qt.ShiftModifier, disposition: NavigationRequest.DispositionNewWindow, current: false },
        { link: "#button1", url: "http://localhost:8080/empty.html", modifiers: Qt.ControlModifier, disposition: NavigationRequest.DispositionNewBackgroundTab, current: false },
        { link: "#button1", url: "http://localhost:8080/empty.html", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NavigationRequest.DispositionNewForegroundTab, current: false },
        { link: "#link2", url: "http://localhost:8080/empty.html", modifiers: Qt.NoModifier, disposition: NavigationRequest.DispositionNewForegroundTab, current: false },
        { link: "#link2", url: "http://localhost:8080/empty.html", modifiers: Qt.ShiftModifier, disposition: NavigationRequest.DispositionNewWindow, current: false },
        { link: "#link2", url: "http://localhost:8080/empty.html", modifiers: Qt.ControlModifier, disposition: NavigationRequest.DispositionNewBackgroundTab, current: false },
        { link: "#link2", url: "http://localhost:8080/empty.html", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NavigationRequest.DispositionNewForegroundTab, current: false },
        // { link: "#button2", url: "http://localhost:8080/empty.html", modifiers: Qt.NoModifier, disposition: NavigationRequest.DispositionNewPopup, current: false },
        // { link: "#button2", url: "http://localhost:8080/empty.html", modifiers: Qt.ShiftModifier, disposition: NavigationRequest.DispositionNewPopup, current: false },
        { link: "#button2", url: "http://localhost:8080/empty.html", modifiers: Qt.ControlModifier, disposition: NavigationRequest.DispositionNewBackgroundTab, current: false },
        { link: "#button2", url: "http://localhost:8080/empty.html", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NavigationRequest.DispositionNewForegroundTab, current: false },
      ];
    }

    // Test that we get an onNavigationRequested signal for all renderer-initiated
    // top-level navigations (also verifies that we don't get one for browser-
    // initiated navigations)
    function test_NavigationRequest1_from_user_gestures(data) {
      webView.url = "http://localhost:8080/tst_NavigationRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(spy.count, 0,
              "Shouldn't get an onNavigationRequested signal for browser-initiated navigation");

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.link);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, data.modifiers);

      if (data.disposition == NavigationRequest.DispositionCurrentTab) {
        verify(webView.waitForLoadSucceeded());
      } else {
        newViewSpy.wait();
      }

      compare(spy.count, 1, "Should have had an onNavigationRequested signal");
      compare(webView.lastRequestUrl, data.url);
      compare(webView.lastRequestDisposition, data.disposition);
      compare(webView.lastRequestUserGesture, true);
    }

    function test_NavigationRequest2_no_user_gesture_data() {
      return test_NavigationRequest1_from_user_gestures_data();
    }

    // Verify that the userGesture property indicates the appropriate value
    // for renderer-initiated top-level navigations that don't come from an
    // input event
    function test_NavigationRequest2_no_user_gesture(data) {
      webView.context.popupBlockerEnabled = false;

      webView.url = "http://localhost:8080/tst_NavigationRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(spy.count, 0,
              "Shouldn't get an onNavigationRequested signal for browser-initiated navigation");

      webView.getTestApi().evaluateCode(
"var e = document.createEvent(\"HTMLEvents\");
e.initEvent(\"click\", true, false);
document.querySelector(\"" + data.link + "\").dispatchEvent(e);", true);

      if (data.current) {
        verify(webView.waitForLoadSucceeded());
      } else {
        newViewSpy.wait();
      }

      compare(spy.count, 1, "Should have had an onNavigationRequested signal")
      compare(webView.lastRequestUrl, "http://localhost:8080/empty.html");
      compare(webView.lastRequestDisposition, data.current ? NavigationRequest.DispositionCurrentTab : NavigationRequest.DispositionNewPopup );
      compare(webView.lastRequestUserGesture, false);
    }

    function test_NavigationRequest3_reject_data() {
      return test_NavigationRequest1_from_user_gestures_data();
    }

    // Verify that rejecting an onNavigationRequested request for all
    // renderer-initiated top-level navigations blocks the navigation and that
    // we don't get any onNewViewRequested signals.
    //
    // XXX(chrisccoulson): This is a bit hacky, because we use a 100ms delay
    // before verifying no loads started
    function test_NavigationRequest3_reject(data) {
      webView.shouldReject = true;

      webView.url = "http://localhost:8080/tst_NavigationRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.clearLoadEventCounters();

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.link);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, data.modifiers);
      webView.waitFor(function() { return false; }, 100);

      compare(spy.count, 1);
      compare(newViewSpy.count, 0, "Shouldn't have called onNewViewRequested for rejected navigation");
      compare(webView.loadsStartedCount, 0, "Shouldn't have started a load for rejected navigation");
    }

    function test_NavigationRequest4_subframe_data() {
      return test_NavigationRequest1_from_user_gestures_data();
    }

    // Verify that we don't get an onNavigationRequested signal for
    // renderer-initiated subframe navigations unless the disposition is not
    // DispositionCurrentTab.
    // We get them for other dispositions via
    // content::RenderFrameImpl::loadURLExternally(), which is called from
    // WebCore::DocumentLoader::shouldContinueForNavigationPolicy()
    function test_NavigationRequest4_subframe(data) {
      webView.url = "http://localhost:8080/tst_NavigationRequest2.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var frame = webView.rootFrame.childFrames[0];

      frameSpy.target = frame;
      var r = webView.getTestApiForFrame(frame).getBoundingClientRectForSelector(data.link);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, data.modifiers);

      if (data.disposition == NavigationRequest.DispositionCurrentTab) {
        frameSpy.wait();
        compare(spy.count, 0, "Shouldn't get onNavigationRequested from CurrentTab subframe navigations");
      } else {
        newViewSpy.wait();
        compare(spy.count, 1, "Should get onNavigationRequested from non-CurrentTab subframe navigations");
      }
    }
  }
}
