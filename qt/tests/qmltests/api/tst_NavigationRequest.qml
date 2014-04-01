import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

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
    }

    function test_NavigationRequest1_from_user_gestures_data() {
      return [
        { link: "#link1", url: "http://localhost:8888/", local: false, modifiers: Qt.NoModifier, disposition: NavigationRequest.DispositionCurrentTab },
        { link: "#link1", url: "http://localhost:8888/", local: false, modifiers: Qt.ShiftModifier, disposition: NavigationRequest.DispositionNewWindow },
        { link: "#link1", url: "http://localhost:8888/", local: false, modifiers: Qt.ControlModifier, disposition: NavigationRequest.DispositionNewBackgroundTab },
        { link: "#link1", url: "http://localhost:8888/", local: false, modifiers: Qt.ShiftModifier | Qt.ControlModifier, disposition: NavigationRequest.DispositionNewForegroundTab },
        { link: "#link2", url: "http://localhost:8080/empty.html", local: true, modifiers: Qt.NoModifier, request: false, disposition: NavigationRequest.DispositionCurrentTab },
        { link: "#link2", url: "http://localhost:8080/empty.html", local: true, modifiers: Qt.ShiftModifier, request: false, disposition: NavigationRequest.DispositionNewWindow },
        { link: "#link2", url: "http://localhost:8080/empty.html", local: true, modifiers: Qt.ControlModifier, request: false, disposition: NavigationRequest.DispositionNewBackgroundTab },
        { link: "#link2", url: "http://localhost:8080/empty.html", local: true, modifiers: Qt.ShiftModifier | Qt.ControlModifier, request: false, disposition: NavigationRequest.DispositionNewForegroundTab }
      ];
    }

    function test_NavigationRequest1_from_user_gestures(data) {
      webView.url = "http://localhost:8080/tst_NavigationRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.link);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, data.modifiers);

      if (data.disposition == NavigationRequest.DispositionCurrentTab) {
        verify(webView.waitForLoadSucceeded());
        compare(newViewSpy.count, 0);
      } else {
        newViewSpy.wait();
      }

      compare(spy.count, 1, "Should have had a signal");
      compare(webView.lastRequestUrl, data.url);
      compare(webView.lastRequestDisposition, data.disposition);
      compare(webView.lastRequestUserGesture, true);
    }

    function test_NavigationRequest2_no_user_gesture_data() {
      return test_NavigationRequest1_from_user_gestures_data();
    }

    function test_NavigationRequest2_no_user_gesture(data) {
      webView.url = "http://localhost:8080/tst_NavigationRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"var e = document.createEvent(\"HTMLEvents\");
e.initEvent(\"click\", true, false);
document.querySelector(\"" + data.link + "\").dispatchEvent(e);", true);

      verify(webView.waitForLoadSucceeded());
      compare(newViewSpy.count, 0);

      compare(spy.count, 1, "Should have had a signal");
      compare(webView.lastRequestUrl, data.url);
      compare(webView.lastRequestDisposition, NavigationRequest.DispositionCurrentTab);
      compare(webView.lastRequestUserGesture, false);
    }

    function test_NavigationRequest3_reject_data() {
      return test_NavigationRequest1_from_user_gestures_data();
    }

    function test_NavigationRequest3_reject(data) {
      webView.shouldReject = true;

      webView.url = "http://localhost:8080/tst_NavigationRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.clearLoadEventCounters();

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.link);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, data.modifiers);
      webView.waitFor(function() { return false; }, 100); // XXX: Is there a better way of doing this?

      compare(spy.count, 1);
      compare(newViewSpy.count, 0, "Shouldn't have called onNewViewRequested for rejected navigation");
      compare(webView.loadsStartedCount, 0, "Shouldn't have started a load for rejected navigation");
    }

    function test_NavigationRequest4_subframe_data() {
      return test_NavigationRequest1_from_user_gestures_data();
    }

    function test_NavigationRequest4_subframe(data) {
      webView.url = "http://localhost:8080/tst_NavigationRequest2.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      console.log(spy.count);
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
