import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  property var lastGeolocationRequest: null

  onGeolocationPermissionRequested: {
    lastGeolocationRequest = request;
  }

  property var lastGeolocationStatus: -1

  messageHandlers: [
    ScriptMessageHandler {
      msgId: "GEOLOCATION-RESPONSE"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        webView.lastGeolocationStatus = msg.args.status;
      }
    }
  ]

  TestCase {
    id: test
    name: "GeolocationPermissionRequest"
    when: windowShown

    function init() {
      webView.lastGeolocationRequest = null;
      webView.lastGeolocationStatus = -1;
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

    function test_GeolocationPermissionRequest1_data() {
      return [
        { function: _test_accept, expected: 0 },
        { function: _test_deny, expected: 1 },
        { function: _test_destroy, expected: 1 }
      ];
    }

    SignalSpy {
      id: spy
      signalName: "cancelled"
    }

    function test_GeolocationPermissionRequest1(data) {
      webView.url = "http://localhost:8080/tst_GeolocationPermissionRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"document.addEventListener(\"oxidegeolocationresult\", function(event) {
  oxide.sendMessage(\"GEOLOCATION-RESPONSE\", { status: event.detail.status });
});", true);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      verify(webView.waitFor(function() { return !!webView.lastGeolocationRequest; }),
             "Timed out waiting for geolocation request");
      compare(webView.lastGeolocationRequest.origin, "http://localhost:8080/");
      compare(webView.lastGeolocationRequest.embedder, "http://localhost:8080/");
      compare(webView.lastGeolocationRequest.isCancelled, false);

      data.function();

      verify(webView.waitFor(function() { return webView.lastGeolocationStatus != -1; }),
             "Timed out waiting for geolocation response");
      compare(webView.lastGeolocationStatus, data.expected);
    }

    function test_GeolocationPermissionRequest2_cancel() {
      webView.url = "http://localhost:8080/tst_GeolocationPermissionRequest.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      verify(webView.waitFor(function() { return !!webView.lastGeolocationRequest; }),
             "Timed out waiting for geolocation request");

      spy.target = webView.lastGeolocationRequest;

      webView.getTestApi().evaluateCode(
          "window.location = \"http://localhost:8080/empty.html\";", false);

      spy.wait();
      compare(spy.count, 1) << "Pending request should have been cancelled";
    }
  }
}
