import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.11
import com.canonical.Oxide.Testing 1.0

Item {
  width: 200
  height: 200

  SignalSpy {
    id: spy
    signalName: "notificationPermissionRequested"
  }

  Component {
    id: userScriptFactory
    UserScript {}
  }

  TestWebContext {
    id: c
    Component.onCompleted: {
      var script = userScriptFactory.createObject(null, {
          context: "oxide://testutils/",
          url: Qt.resolvedUrl("tst_WebNotificationPermissionRequest.js"),
          matchAllFrames: true
      });
      addUserScript(script);
    }
  }

  Component {
    id: webViewFactory
    TestWebView {
      context: c

      property var lastRequest: null
      onNotificationPermissionRequested: {
        lastRequest = request;
      }

      QtObject {
        id: _internal
        property var lastStatus: -1
      }
      property alias lastStatus: _internal.lastStatus

      messageHandlers: [
        ScriptMessageHandler {
          msgId: "TEST-RESPONSE"
          contexts: [ "oxide://testutils/" ]
          callback: function(msg) {
            _internal.lastStatus = msg.args.status;
          }
        }
      ]
    }
  }

  TestCase {
    id: test
    name: "NotificationPermissionRequest_session_persist"
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
      spy.target = null;
      spy.clear();
      c.clearTemporarySavedPermissionStatuses();
    }

    function test_NotificationPermissionRequest_session_persist1_data() {
      return [
        { function: _test_accept, expected: 0, save: true },
        { function: _test_deny, expected: 1, save: true },
        { function: _test_destroy, expected: 1, save: false },
      ];
    }

    function test_NotificationPermissionRequest_session_persist1(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = "http://foo.testsuite/tst_WebNotificationPermissionRequest.html";

      verify(webView.waitForLoadSucceeded());

      spy.wait();

      compare(webView.lastRequest.embedder, "http://foo.testsuite/");

      data.function(webView.lastRequest);

      verify(webView.waitFor(function() { return webView.lastStatus != -1; }));
      compare(webView.lastStatus, data.expected);

      spy.clear();
      webView.lastStatus = -1;

      webView.reload();
      verify(webView.waitForLoadSucceeded());

      if (data.save) {
        verify(webView.waitFor(function() { return webView.lastStatus != -1; }));
        compare(webView.lastStatus, data.expected);
      } else {
        spy.wait();
        compare(webView.lastRequest.embedder, "http://foo.testsuite/");
      }
    }

    function test_NotificationPermissionRequest_session_persist2_data() {
      return test_NotificationPermissionRequest_session_persist1_data();
    }

    function test_NotificationPermissionRequest_session_persist2(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = "http://foo.testsuite/tst_WebNotificationPermissionRequest.html";
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      compare(webView.lastRequest.embedder, "http://foo.testsuite/");

      data.function(webView.lastRequest);

      verify(webView.waitFor(function() { return webView.lastStatus != -1; }));
      compare(webView.lastStatus, data.expected);

      webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      spy.clear();

      webView.url = "http://foo.testsuite/tst_WebNotificationPermissionRequest.html";
      verify(webView.waitForLoadSucceeded());

      if (data.save) {
        verify(webView.waitFor(function() { return webView.lastStatus != -1; }));
        compare(webView.lastStatus, data.expected);
      } else {
        spy.wait();
        compare(webView.lastRequest.embedder, "http://foo.testsuite/");
      }
    }

    function test_NotificationPermissionRequest_session_persist3_data() {
      return [
        // Same origin / different embedder
        { url1: "http://foo.testsuite/tst_WebNotificationPermissionRequest.html",
          url2: "http://bar.testsuite/tst_WebNotificationPermissionRequest.html" },
        // Same origin / embedder == allowed origin
        { url1: "http://foo.testsuite/tst_WebNotificationPermissionRequest.html",
          url2: "http://testsuite/tst_WebNotificationPermissionRequest.html" },
        // Different origin / different embedder
        { url1: "http://foo.testsuite/tst_WebNotificationPermissionRequest.html",
          url2: "http://bar.testsuite/tst_WebNotificationPermissionRequest.html" },
      ];
    }

    // Verify several cases where saved permissions should not be used
    function test_NotificationPermissionRequest_session_persist3(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = data.url1;
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      webView.lastRequest.allow();

      verify(webView.waitFor(function() { return webView.lastStatus != -1; }));
      compare(webView.lastStatus, 0);

      spy.clear();
      webView.lastStatus = -1;

      webView.url = data.url2;
      verify(webView.waitForLoadSucceeded());

      spy.wait();
    }
  }
}
