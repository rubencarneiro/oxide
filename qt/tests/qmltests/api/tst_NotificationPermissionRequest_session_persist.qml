import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

Item {
  id: top
  width: 200
  height: 200

  TestWebContext {
    id: c
    Component.onCompleted: {
      addTestUserScript({
          context: "oxide://notifytest/",
          url: Qt.resolvedUrl("tst_NotificationPermissionRequest_session_persist.js"),
          incognitoEnabled: true,
          matchAllFrames: true
      });
    }
  }

  SignalSpy {
    id: spy
    signalName: "notificationPermissionRequested"
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
          contexts: [ "oxide://notifytest/" ]
          callback: function(msg) {
            _internal.lastStatus = msg.payload;
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

    // Verify that notification permission request decisions are remembered by
    // responding to an initial request and then reloading the webview
    function test_NotificationPermissionRequest_session_persist1(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = "http://foo.testsuite/tst_NotificationPermissionRequest_session_persist_embedder.html";
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      compare(webView.lastRequest.origin, "http://testsuite/");
      compare(webView.lastRequest.embedder, "http://foo.testsuite/");

      data.function(webView.lastRequest);

      verify(TestUtils.waitFor(function() { return webView.lastStatus != -1; }));
      compare(webView.lastStatus, data.expected);

      spy.clear();
      webView.lastStatus = -1;

      webView.reload();
      verify(webView.waitForLoadSucceeded());

      if (data.save) {
        verify(TestUtils.waitFor(function() { return webView.lastStatus != -1; }));
        compare(webView.lastStatus, data.expected);
      } else {
        spy.wait();
        compare(webView.lastRequest.origin, "http://testsuite/");
        compare(webView.lastRequest.embedder, "http://foo.testsuite/");
      }
    }

    function test_NotificationPermissionRequest_session_persist2_data() {
      return test_NotificationPermissionRequest_session_persist1_data();
    }

    // Verify that notification permission request decisions are remembered by
    // responding to an initial request in one webview, and then attempting the
    // same access in a new webview
    function test_NotificationPermissionRequest_session_persist2(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = "http://foo.testsuite/tst_NotificationPermissionRequest_session_persist_embedder.html";
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      compare(webView.lastRequest.origin, "http://testsuite/");
      compare(webView.lastRequest.embedder, "http://foo.testsuite/");

      data.function(webView.lastRequest);

      verify(TestUtils.waitFor(function() { return webView.lastStatus != -1; }));
      compare(webView.lastStatus, data.expected);

      webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      spy.clear();

      webView.url = "http://foo.testsuite/tst_NotificationPermissionRequest_session_persist_embedder.html";
      verify(webView.waitForLoadSucceeded());

      if (data.save) {
        verify(TestUtils.waitFor(function() { return webView.lastStatus != -1; }));
        compare(webView.lastStatus, data.expected);
      } else {
        spy.wait();
        compare(webView.lastRequest.origin, "http://testsuite/");
        compare(webView.lastRequest.embedder, "http://foo.testsuite/");
      }
    }

    function test_NotificationPermissionRequest_session_persist3_data() {
      return [
        // Same origin / different embedder
        // This case doesn't work for notifications, as the embedder isn't used for saved permissions
        //{ url1: "http://foo.testsuite/tst_NotificationPermissionRequest_session_persist_embedder.html",
        //  url2: "http://bar.testsuite/tst_NotificationPermissionRequest_session_persist_embedder.html" },
        // Same origin / embedder == allowed origin
        // This case doesn't work for notifications, as the embedder isn't used for saved permissions
        //{ url1: "http://foo.testsuite/tst_NotificationPermissionRequest_session_persist_embedder.html",
        //  url2: "http://testsuite/tst_NotificationPermissionRequest_session_persist.html" },
        // Different origin / same embedder
        { url1: "http://foo.testsuite/tst_NotificationPermissionRequest_session_persist.html",
          url2: "http://foo.testsuite/tst_NotificationPermissionRequest_session_persist_embedder.html" },
        // Different origin / different embedder
        { url1: "http://foo.testsuite/tst_NotificationPermissionRequest_session_persist.html",
          url2: "http://bar.testsuite/tst_NotificationPermissionRequest_session_persist_embedder.html" },
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

      verify(TestUtils.waitFor(function() { return webView.lastStatus != -1; }));
      compare(webView.lastStatus, 0);

      spy.clear();
      webView.lastStatus = -1;

      webView.url = data.url2;
      verify(webView.waitForLoadSucceeded());

      spy.wait();
    }

    function test_NotificationPermissionRequest_session_persist4_data() {
      return [
        { url1: "http://testsuite/tst_NotificationPermissionRequest_session_persist_embedder.html",
          url2: "http://testsuite/tst_NotificationPermissionRequest_session_persist.html" },
        { url1: "http://testsuite/tst_NotificationPermissionRequest_session_persist.html",
          url2: "http://testsuite/tst_NotificationPermissionRequest_session_persist_embedder.html" },
      ];
    }

    // Verify cases where saved permissions should be used
    function test_NotificationPermissionRequest_session_persist4(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = data.url1;
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      webView.lastRequest.allow();

      verify(TestUtils.waitFor(function() { return webView.lastStatus != -1; }));
      compare(webView.lastStatus, 0);

      spy.clear();
      webView.lastStatus = -1;

      webView.url = data.url2;
      verify(webView.waitForLoadSucceeded());

      verify(TestUtils.waitFor(function() { return webView.lastStatus != -1; }));
      compare(webView.lastStatus, 0);
    }
  }
}
