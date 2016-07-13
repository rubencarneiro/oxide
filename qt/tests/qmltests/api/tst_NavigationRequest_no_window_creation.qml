import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView

  property string lastRequestUrl: ""
  property int lastRequestDisposition: NavigationRequest.DispositionCurrentTab
  property bool lastRequestUserGesture: false

  onNavigationRequested: {
    lastRequestUrl = request.url;
    lastRequestDisposition = request.disposition;
    lastRequestUserGesture = request.userGesture;
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "navigationRequested"
  }

  SignalSpy {
    id: frameSpy
    signalName: "urlChanged"
  }

  TestCase {
    id: test
    name: "NavigationRequest_no_window_creation"
    when: windowShown

    function init() {
      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      spy.clear();
      frameSpy.clear();
    }

    function test_NavigationRequest_no_window_creation1_data() {
      return [
        { link: "#link1", url: "http://testsuite/empty.html", modifiers: Qt.NoModifier, current: true },
        { link: "#link1", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifier, brokenUserGesture: true },
        { link: "#link1", url: "http://testsuite/empty.html", modifiers: Qt.ControlModifier, brokenUserGesture: true },
        { link: "#link1", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifier | Qt.ControlModifier, brokenUserGesture: true },
        { link: "#button1", url: "http://testsuite/empty.html", modifiers: Qt.NoModifier },
        { link: "#button1", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifier },
        { link: "#button1", url: "http://testsuite/empty.html", modifiers: Qt.ControlModifier },
        { link: "#button1", url: "http://testsuite/empty.html", modifiers: Qt.ShiftMofifier | Qt.ControlModifier },
        { link: "#link2", url: "http://testsuite/empty.html", modifiers: Qt.NoModifier },
        { link: "#link2", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifier },
        { link: "#link2", url: "http://testsuite/empty.html", modifiers: Qt.ControlModifier },
        { link: "#link2", url: "http://testsuite/empty.html", modifiers: Qt.ShiftMofifier | Qt.ControlModifier },
        { link: "#button2", url: "http://testsuite/empty.html", modifiers: Qt.NoModifier },
        { link: "#button2", url: "http://testsuite/empty.html", modifiers: Qt.ShiftModifiere },
        { link: "#button2", url: "http://testsuite/empty.html", modifiers: Qt.ControlModifier },
        { link: "#button2", url: "http://testsuite/empty.html", modifiers: Qt.ShiftMofifier | Qt.ControlModifier },
      ];
    }

    // Test that we get an onNavigationRequested signal for all renderer-initiated
    // top-level navigations (also verifies that we don't get one for browser-
    // initiated navigations)
    function test_NavigationRequest_no_window_creation1(data) {
      webView.url = "http://testsuite/tst_NavigationRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(spy.count, 0,
              "Shouldn't get an onNavigationRequested signal for browser-initiated navigation");

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.link);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, data.modifiers);

      verify(webView.waitForLoadSucceeded());

      compare(spy.count, 1, "Should have had an onNavigationRequested signal");
      compare(webView.lastRequestUrl, data.url);
      compare(webView.lastRequestDisposition, NavigationRequest.DispositionCurrentTab);
      // NavigationRequest.userGesture is broken for CURRENT_TAB navigations started via WebView::OpenURLFromTab
      // See https://launchpad.net/bugs/1499434
      compare(webView.lastRequestUserGesture, data.brokenUserGesture ? false : true);
    }

    function test_NavigationRequest_no_window_creation2_subframe_data() {
      return test_NavigationRequest_no_window_creation1_data();
    }

    // XXX(chrisccoulson): Disabled due to https://launchpad.net/bugs/1302740
    function test_NavigationRequest_no_window_creation2_subframe(data) {
      webView.url = "http://testsuite/tst_NavigationRequest2.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var frame = webView.rootFrame.childFrames[0];
      frameSpy.target = frame;

      var r = webView.getTestApiForFrame(frame).getBoundingClientRectForSelector(data.link);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, data.modifiers);

      if (data.current) {
        frameSpy.wait();
      } else {
        verify(webView.waitForLoadSucceeded());
      }

      compare(spy.count, data.current ? 0 : 1);
      if (!data.current) {
        compare(webView.lastRequestUrl, data.url);
        compare(webView.lastRequestDisposition, NavigationRequest.DispositionCurrentTab);
        // NavigationRequest.userGesture is broken for CURRENT_TAB navigations started via WebView::OpenURLFromTab
        // See https://launchpad.net/bugs/1499434
        compare(webView.lastRequestUserGesture, data.brokenUserGesture ? false : true);
      }
    }
  }
}
