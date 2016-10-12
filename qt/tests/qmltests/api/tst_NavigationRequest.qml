import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

// FIXME: Test navigations in a subwindow (ie, one opened with window.open() and with
// window.opener pointing to its parent - we should only get onNavigationRequest
// notifications for cross-domain navigations as opposed to all navigations)

TestWebView {
  id: webView
  focus: true

  property string lastRequestUrl: ""
  property int lastRequestDisposition: NavigationRequest.DispositionCurrentTab
  property bool lastRequestUserGesture: false
  property bool lastRequestLoadingState: false

  property bool shouldReject: false

  onNavigationRequested: {
    if (shouldReject) {
      request.action = NavigationRequest.ActionReject;
      return;
    }

    lastRequestUrl = request.url;
    lastRequestDisposition = request.disposition;
    lastRequestUserGesture = request.userGesture;
    lastRequestLoadingState = webView.loading;
  }

  Component {
    id: webViewFactory
    WebView {}
  }

  onNewViewRequested: {
    webViewFactory.createObject(webView, { request: request });
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
      webView.clearLoadEventCounters();
    }

    function cleanupTestCase() {
      webView.context.popupBlockerEnabled = true;
    }

    function test_NavigationRequest1_from_user_gestures_data() {
      return [
        { link: "#link1", url: "http://testsuite/empty.html", modifiers: Qt.NoModifier, disposition: NavigationRequest.DispositionCurrentTab, current: true },
        { link: "#link1", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifier, disposition: NavigationRequest.DispositionNewWindow, current: true },
        { link: "#link1", url: "http://testsuite/empty.html", modifiers: Qt.ControlModifier, disposition: NavigationRequest.DispositionNewBackgroundTab, current: true },
        { link: "#link1", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NavigationRequest.DispositionNewForegroundTab, current: true },
        { link: "#button1", url: "http://testsuite/empty.html", modifiers: Qt.NoModifier, disposition: NavigationRequest.DispositionNewForegroundTab, current: false },
        { link: "#button1", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifier, disposition: NavigationRequest.DispositionNewWindow, current: false },
        { link: "#button1", url: "http://testsuite/empty.html", modifiers: Qt.ControlModifier, disposition: NavigationRequest.DispositionNewBackgroundTab, current: false },
        { link: "#button1", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NavigationRequest.DispositionNewForegroundTab, current: false },
        { link: "#link2", url: "http://testsuite/empty.html", modifiers: Qt.NoModifier, disposition: NavigationRequest.DispositionNewForegroundTab, current: false },
        { link: "#link2", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifier, disposition: NavigationRequest.DispositionNewWindow, current: false },
        { link: "#link2", url: "http://testsuite/empty.html", modifiers: Qt.ControlModifier, disposition: NavigationRequest.DispositionNewBackgroundTab, current: false },
        { link: "#link2", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NavigationRequest.DispositionNewForegroundTab, current: false },
        // XXX(chrisccoulson): These 2 disabled due to https://launchpad.net/bugs/1302743
        // { link: "#button2", url: "http://testsuite/empty.html", modifiers: Qt.NoModifier, disposition: NavigationRequest.DispositionNewPopup, current: false },
        // { link: "#button2", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifier, disposition: NavigationRequest.DispositionNewPopup, current: false },
        { link: "#button2", url: "http://testsuite/empty.html", modifiers: Qt.ControlModifier, disposition: NavigationRequest.DispositionNewBackgroundTab, current: false },
        { link: "#button2", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NavigationRequest.DispositionNewForegroundTab, current: false },
      ];
    }

    // Test that we get an onNavigationRequested signal for all renderer-initiated
    // top-level navigations (also verifies that we don't get one for browser-
    // initiated navigations)
    function test_NavigationRequest1_from_user_gestures(data) {
      webView.url = "http://testsuite/tst_NavigationRequest.html";
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
      compare(webView.lastRequestLoadingState, data.disposition == NavigationRequest.DispositionCurrentTab ? true : false);
    }

    function test_NavigationRequest2_no_user_gesture_data() {
      return test_NavigationRequest1_from_user_gestures_data();
    }

    // Verify that the userGesture property indicates the appropriate value
    // for renderer-initiated top-level navigations that don't come from an
    // input event
    function test_NavigationRequest2_no_user_gesture(data) {
      webView.url = "http://testsuite/tst_NavigationRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.context.popupBlockerEnabled = false;

      compare(spy.count, 0,
              "Shouldn't get an onNavigationRequested signal for browser-initiated navigation");

      webView.getTestApi().evaluateCode(
"var e = document.createEvent(\"MouseEvent\");
e.initEvent(\"click\", true, false);
document.querySelector(\"" + data.link + "\").dispatchEvent(e);", true);

      if (data.current) {
        verify(webView.waitForLoadSucceeded());
      } else {
        newViewSpy.wait();
      }

      webView.context.popupBlockerEnabled = true;

      compare(spy.count, 1, "Should have had an onNavigationRequested signal")
      compare(webView.lastRequestUrl, "http://testsuite/empty.html");
      compare(webView.lastRequestDisposition, data.current ? NavigationRequest.DispositionCurrentTab : NavigationRequest.DispositionNewPopup );
      compare(webView.lastRequestUserGesture, false);
      compare(webView.lastRequestLoadingState, data.current ? true : false);
    }

    function test_NavigationRequest3_reject_data() {
      return test_NavigationRequest1_from_user_gestures_data();
    }

    // Verify that rejecting an onNavigationRequested request for all
    // renderer-initiated top-level navigations blocks the navigation and that
    // we don't get any onNewViewRequested signals.
    //
    // XXX(chrisccoulson): This is a bit hacky, because we use a 200ms delay
    // before verifying no loads started
    function test_NavigationRequest3_reject(data) {
      webView.shouldReject = true;

      webView.url = "http://testsuite/tst_NavigationRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.clearLoadEventCounters();

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.link);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, data.modifiers);

      spy.wait();
      compare(spy.count, 1);

      if (data.disposition == NavigationRequest.DispositionCurrentTab) {
        verify(webView.waitForLoadStopped());
      } else {
        TestSupport.wait(100);
      }

      compare(newViewSpy.count, 0, "Shouldn't have called onNewViewRequested for rejected navigation");
      compare(webView.loadsStartedCount, data.disposition == NavigationRequest.DispositionCurrentTab ? 1 : 0);
      compare(webView.loadsCommittedCount, 0, "Shouldn't have committed a load for rejected navigation");
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
      webView.url = "http://testsuite/tst_NavigationRequest2.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var frame = webView.rootFrame.childFrames[0];

      frameSpy.target = frame;
      var frameRect = webView.getTestApi().getBoundingClientRectForSelector("iframe");
      var r = webView.getTestApiForFrame(frame).getBoundingClientRectForSelector(data.link);
      mouseClick(webView, (r.x + r.width / 2) + frameRect.x, (r.y + r.height / 2) + frameRect.y, Qt.LeftButton, data.modifiers);

      if (data.disposition == NavigationRequest.DispositionCurrentTab) {
        frameSpy.wait();
        compare(spy.count, 0, "Shouldn't get onNavigationRequested from CurrentTab subframe navigations");
      } else {
        newViewSpy.wait();
        compare(spy.count, 1, "Should get onNavigationRequested from non-CurrentTab subframe navigations");
      }
    }

    // Verify we don't get an onNavigationRequested signal for browser-
    // initiated navigations via WebView.url
    function test_NavigationRequest5_browser_initiated_url() {
      webView.shouldReject = true;
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());
      compare(spy.count, 0);
    }

    // Verify we don't get an onNavigationRequested signal for browser-
    // initiated navigations via WebView.loadHtml()
    function test_NavigationRequest6_browser_initiated_loadHtml() {
      webView.shouldReject = true;
      webView.loadHtml("<html><body><div>FOO</div></body></html>", "file:///");
      verify(webView.waitForLoadSucceeded());
      compare(spy.count, 0);
    }

    // Verify we don't get an onNavigationRequested signal for browser-
    // initiated navigations via WebView.reload()
    function test_NavigationRequest7_browser_initiated_reload() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.shouldReject = true;
      webView.reload();
      verify(webView.waitForLoadSucceeded());
      compare(spy.count, 0);
    }

    // Verify we don't get an onNavigationRequested signal for browser-
    // initiated navigations via WebView.goBack() and WebView.goForward()
    function test_NavigationRequest8_browser_initiated_back_forward() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());
      webView.url = "http://foo.testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.shouldReject = true;
      webView.goBack();
      verify(webView.waitForLoadSucceeded());
      compare(spy.count, 0);

      webView.goForward();
      verify(webView.waitForLoadSucceeded());
      compare(spy.count, 0);
    }
  }
}
