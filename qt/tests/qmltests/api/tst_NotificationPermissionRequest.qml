import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.11
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "notificationPermissionRequested"
  }

  SignalSpy {
    id: cancelSpy
    signalName: "cancelled"
  }

  TestWebContext {
    id: c
    Component.onCompleted: {
      addTestUserScript({
          context: "oxide://notifytest/",
          url: Qt.resolvedUrl("tst_NotificationPermissionRequest.js"),
          matchAllFrames: true
      });
    }
  }

  context: c

  property var lastRequest: null
  onNotificationPermissionRequested: {
    lastRequest = request;
  }

  property var lastStatus: -1
  messageHandlers: [
    ScriptMessageHandler {
      msgId: "TEST-RESPONSE"
      contexts: [ "oxide://notifytest/" ]
      callback: function(msg) {
        webView.lastStatus = msg.payload;
      }
    }
  ]

  TestCase {
    id: test
    name: "NotificationPermissionRequest"
    when: windowShown

    function _test_accept(req) {
      req.allow();
    }

    function _test_deny(req) {
      req.deny();
    }

    function _test_destroy(req) {
      req.destroy();
    }

    function init() {
      spy.clear();
      cancelSpy.clear();
      cancelSpy.target = null;
      c.clearTemporarySavedPermissionStatuses();
      webView.lastRequest = null;
      webView.lastStatus = -1;
    }

    function test_NotificationPermissionRequest1_main_frame_data() {
      return [
        { function: _test_accept, expected: 0 },
        { function: _test_deny, expected: 1 },
        { function: _test_destroy, expected: 1 },
      ];
    }

    function test_NotificationPermissionRequest1_main_frame(data) {
      webView.url = "http://foo.testsuite/tst_NotificationPermissionRequest.html";
      verify(webView.waitForLoadSucceeded());

      if (!webView.lastRequest) {
        spy.wait();
      }

      compare(webView.lastRequest.origin, "http://foo.testsuite/");
      compare(webView.lastRequest.embedder, "http://foo.testsuite/");
      compare(webView.lastRequest.isCancelled, false);

      data.function(webView.lastRequest);

      verify(TestUtils.waitFor(function() { return webView.lastStatus != -1; }));
      compare(webView.lastStatus, data.expected);
    }

    function test_NotificationPermissionRequest2_subframe_data() {
      return test_NotificationPermissionRequest1_main_frame_data();
    }

    function test_NotificationPermissionRequest2_subframe(data) {
      webView.url = "http://foo.testsuite/tst_NotificationPermissionRequest_subframe.html";
      verify(webView.waitForLoadSucceeded());

      if (!webView.lastRequest) {
        spy.wait();
      }

      compare(webView.lastRequest.origin, "http://bar.testsuite/");
      compare(webView.lastRequest.embedder, "http://foo.testsuite/");
      compare(webView.lastRequest.isCancelled, false);

      data.function(webView.lastRequest);

      verify(TestUtils.waitFor(function() { return webView.lastStatus != -1; }));
      compare(webView.lastStatus, data.expected);
    }

    function test_NotificationPermissionRequest3_main_frame_navigation_cancel() {
      webView.url = "http://foo.testsuite/tst_NotificationPermissionRequest.html";
      verify(webView.waitForLoadSucceeded());

      if (!webView.lastRequest) {
        spy.wait();
      }

      cancelSpy.target = webView.lastRequest;

      webView.clearLoadEventCounters();
      webView.getTestApi().evaluateCode(
          "window.location = \"http://testsuite/empty.html\";", false);
      verify(webView.waitForLoadCommitted());

      compare(cancelSpy.count, 1) << "Pending request should have been cancelled";
      verify(webView.lastRequest.isCancelled);
    }

    function test_NotificationPermissionRequest4_subframe_navigation_cancel() {
      webView.url = "http://foo.testsuite/tst_NotificationPermissionRequest_subframe.html";
      verify(webView.waitForLoadSucceeded());

      if (!webView.lastRequest) {
        spy.wait();
      }

      cancelSpy.target = webView.lastRequest;

      webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode(
          "window.location = \"http://testsuite/empty.html\";", false);
 
      cancelSpy.wait();

      compare(cancelSpy.count, 1) << "Pending request should have been cancelled";
      verify(webView.lastRequest.isCancelled);

    }

    function test_NotificationPermissionRequest5_subframe_delete_cancel() {
      webView.url = "http://testsuite/tst_NotificationPermissionRequest_subframe.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      if (!webView.lastRequest) {
        spy.wait();
      }

      cancelSpy.target = webView.lastRequest;

      webView.getTestApi().evaluateCode("
var f = document.getElementsByTagName(\"iframe\")[0];
f.parentElement.removeChild(f);", true);

      cancelSpy.wait();

      compare(cancelSpy.count, 1) << "Pending request should have been cancelled";
      verify(webView.lastRequest.isCancelled);
    }
  }
}
