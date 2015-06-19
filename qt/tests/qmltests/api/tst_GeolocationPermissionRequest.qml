import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  property var lastGeolocationRequest: null

  SignalSpy {
    id: spy
    target: webView
    signalName: "geolocationPermissionRequested"
  }

  SignalSpy {
    id: cancelSpy
    signalName: "cancelled"
  }

  onGeolocationPermissionRequested: {
    lastGeolocationRequest = request;
  }

  property var lastGeolocationStatus: -1

  messageHandlers: [
    ScriptMessageHandler {
      msgId: "GEOLOCATION-RESPONSE"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        webView.lastGeolocationStatus = msg.payload;
      }
    }
  ]

  TestCase {
    id: test
    name: "GeolocationPermissionRequest"
    when: windowShown

    function init() {
      spy.clear();
      cancelSpy.clear();
      webView.lastGeolocationRequest = null;
      webView.lastGeolocationStatus = -1;
      webView.context.clearTemporarySavedPermissionStatuses();
    }

    function _test_allow() {
      webView.lastGeolocationRequest.allow();
    }

    function _test_accept() {
      webView.lastGeolocationRequest.accept();
    }

    function _test_deny() {
      webView.lastGeolocationRequest.deny();
    }

    function _test_destroy() {
      webView.lastGeolocationRequest.destroy();
    }

    function test_GeolocationPermissionRequest1_main_frame_data() {
      return [
        { function: _test_allow, expected: 0 },
        { function: _test_deny, expected: 1 },
        { function: _test_destroy, expected: 1 },
        { function: _test_accept, expected: 0 },
      ];
    }

    function test_GeolocationPermissionRequest1_main_frame(data) {
      webView.url = "http://testsuite/tst_GeolocationPermissionRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"document.addEventListener(\"oxidegeolocationresult\", function(event) {
  oxide.sendMessage(\"GEOLOCATION-RESPONSE\", event.detail.status);
});", true);

      if (!webView.lastGeolocationRequest) {
        spy.wait();
      }

      compare(webView.lastGeolocationRequest.origin, "http://testsuite/");
      compare(webView.lastGeolocationRequest.embedder, "http://testsuite/");
      compare(webView.lastGeolocationRequest.isCancelled, false);

      data.function();

      verify(webView.waitFor(function() { return webView.lastGeolocationStatus != -1; }),
             "Timed out waiting for geolocation response");
      compare(webView.lastGeolocationStatus, data.expected);
    }

    function test_GeolocationPermissionRequest2_subframe_data() {
      return [
        { function: _test_allow, expected: 0 },
        { function: _test_deny, expected: 1 },
        { function: _test_destroy, expected: 1 }
      ];
    }

    function test_GeolocationPermissionRequest2_subframe(data) {
      webView.url = "http://foo.testsuite/tst_GeolocationPermissionRequest_subframe.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode(
"document.addEventListener(\"oxidegeolocationresult\", function(event) {
  oxide.sendMessage(\"GEOLOCATION-RESPONSE\", event.detail.status);
});", true);

      if (!webView.lastGeolocationRequest) {
        spy.wait();
      }

      compare(webView.lastGeolocationRequest.origin, "http://testsuite/");
      compare(webView.lastGeolocationRequest.embedder, "http://foo.testsuite/");
      compare(webView.lastGeolocationRequest.isCancelled, false);

      data.function();

      verify(webView.waitFor(function() { return webView.lastGeolocationStatus != -1; }),
             "Timed out waiting for geolocation response");
      compare(webView.lastGeolocationStatus, data.expected);
    }

    function test_GeolocationPermissionRequest3_main_frame_navigation_cancel() {
      webView.url = "http://testsuite/tst_GeolocationPermissionRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      if (!webView.lastGeolocationRequest) {
        spy.wait();
      }

      cancelSpy.target = webView.lastGeolocationRequest;

      webView.clearLoadEventCounters();
      webView.getTestApi().evaluateCode(
          "window.location = \"http://testsuite/empty.html\";", false);
      verify(webView.waitForLoadCommitted());

      compare(cancelSpy.count, 1) << "Pending request should have been cancelled";
      verify(webView.lastGeolocationRequest.isCancelled);
    }

    function test_GeolocationPermissionRequest4_subframe_navigation_cancel() {
      webView.url = "http://testsuite/tst_GeolocationPermissionRequest_subframe.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      if (!webView.lastGeolocationRequest) {
        spy.wait();
      }

      cancelSpy.target = webView.lastGeolocationRequest;

      webView.clearLoadEventCounters();
      webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode(
          "window.location = \"http://testsuite/empty.html\";", false);
 
      cancelSpy.wait();

      compare(cancelSpy.count, 1) << "Pending request should have been cancelled";
      verify(webView.lastGeolocationRequest.isCancelled);
    }

    function test_GeolocationPermissionRequest5_subframe_delete_cancel() {
      webView.url = "http://testsuite/tst_GeolocationPermissionRequest_subframe.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      if (!webView.lastGeolocationRequest) {
        spy.wait();
      }

      cancelSpy.target = webView.lastGeolocationRequest;

      webView.getTestApi().evaluateCode("
var f = document.getElementsByTagName(\"iframe\")[0];
f.parentElement.removeChild(f);", true);

      cancelSpy.wait();

      compare(cancelSpy.count, 1) << "Pending request should have been cancelled";
      verify(webView.lastGeolocationRequest.isCancelled);
    }
  }
}
