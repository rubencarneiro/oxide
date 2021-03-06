import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

Item {
  id: top

  Component.onCompleted: {
    SingletonTestWebContext.addTestUserScript({
        context: "oxide://geotest/",
        url: Qt.resolvedUrl("tst_GeolocationPermissionRequest_session_persist.js"),
        incognitoEnabled: true,
        matchAllFrames: true
    });
  }

  SignalSpy {
    id: spy
    signalName: "geolocationPermissionRequested"
  }

  Component {
    id: webViewFactory
    TestWebView {
      property var lastGeolocationRequest: null
      onGeolocationPermissionRequested: {
        lastGeolocationRequest = request;
      }

      QtObject {
        id: _internal
        property var lastGeolocationStatus: -1
      }
      property alias lastGeolocationStatus: _internal.lastGeolocationStatus

      messageHandlers: [
        ScriptMessageHandler {
          msgId: "GEOLOCATION-RESPONSE"
          contexts: [ "oxide://geotest/" ]
          callback: function(msg) {
            _internal.lastGeolocationStatus = msg.payload;
          }
        }
      ]
    }
  }

  TestCase {
    id: test
    name: "GeolocationPermissionRequest_session_persist"
    when: windowShown

    function cleanupTestCase() {
      SingletonTestWebContext.clearTestUserScripts();
    }

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
      SingletonTestWebContext.clearTemporarySavedPermissionStatuses();
    }

    function test_GeolocationPermissionRequest_session_persist1_data() {
      return [
        { function: _test_accept, expected: 0, save: true },
        { function: _test_deny, expected: 1, save: true },
        { function: _test_destroy, expected: 1, save: false },
      ];
    }

    // Verify that geolocation permission request decisions are remembered by
    // responding to an initial request and then reloading the webview
    function test_GeolocationPermissionRequest_session_persist1(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = "https://foo.testsuite/tst_GeolocationPermissionRequest_session_persist_embedder.html";
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      compare(webView.lastGeolocationRequest.origin, "https://testsuite/");
      compare(webView.lastGeolocationRequest.embedder, "https://foo.testsuite/");

      data.function(webView.lastGeolocationRequest);

      verify(TestUtils.waitFor(function() { return webView.lastGeolocationStatus != -1; }));
      compare(webView.lastGeolocationStatus, data.expected);

      spy.clear();
      webView.lastGeolocationStatus = -1;

      webView.reload();
      verify(webView.waitForLoadSucceeded());

      if (data.save) {
        verify(TestUtils.waitFor(function() { return webView.lastGeolocationStatus != -1; }));
        compare(webView.lastGeolocationStatus, data.expected);
      } else {
        spy.wait();
        compare(webView.lastGeolocationRequest.origin, "https://testsuite/");
        compare(webView.lastGeolocationRequest.embedder, "https://foo.testsuite/");
      }
    }

    function test_GeolocationPermissionRequest_session_persist2_data() {
      return test_GeolocationPermissionRequest_session_persist1_data();
    }

    // Verify that geolocation permission request decisions are remembered by
    // responding to an initial request in one webview, and then attempting the
    // same access in a new webview
    function test_GeolocationPermissionRequest_session_persist2(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = "https://foo.testsuite/tst_GeolocationPermissionRequest_session_persist_embedder.html";
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      compare(webView.lastGeolocationRequest.origin, "https://testsuite/");
      compare(webView.lastGeolocationRequest.embedder, "https://foo.testsuite/");

      data.function(webView.lastGeolocationRequest);

      verify(TestUtils.waitFor(function() { return webView.lastGeolocationStatus != -1; }));
      compare(webView.lastGeolocationStatus, data.expected);

      webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      spy.clear();

      webView.url = "https://foo.testsuite/tst_GeolocationPermissionRequest_session_persist_embedder.html";
      verify(webView.waitForLoadSucceeded());

      if (data.save) {
        verify(TestUtils.waitFor(function() { return webView.lastGeolocationStatus != -1; }));
        compare(webView.lastGeolocationStatus, data.expected);
      } else {
        spy.wait();
        compare(webView.lastGeolocationRequest.origin, "https://testsuite/");
        compare(webView.lastGeolocationRequest.embedder, "https://foo.testsuite/");
      }
    }

    function test_GeolocationPermissionRequest_session_persist3_data() {
      return [
        // Same origin / different embedder
        { url1: "https://foo.testsuite/tst_GeolocationPermissionRequest_session_persist_embedder.html",
          url2: "https://bar.testsuite/tst_GeolocationPermissionRequest_session_persist_embedder.html" },
        // Same origin / embedder == allowed origin
        { url1: "https://foo.testsuite/tst_GeolocationPermissionRequest_session_persist_embedder.html",
          url2: "https://testsuite/tst_GeolocationPermissionRequest_session_persist.html" },
        // Different origin / same embedder
        { url1: "https://foo.testsuite/tst_GeolocationPermissionRequest_session_persist.html",
          url2: "https://foo.testsuite/tst_GeolocationPermissionRequest_session_persist_embedder.html" },
        // Different origin / different embedder
        { url1: "https://foo.testsuite/tst_GeolocationPermissionRequest_session_persist.html",
          url2: "https://bar.testsuite/tst_GeolocationPermissionRequest_session_persist_embedder.html" },
      ];
    }

    // Verify several cases where saved permissions should not be used
    function test_GeolocationPermissionRequest_session_persist3(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = data.url1;
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      webView.lastGeolocationRequest.allow();

      verify(TestUtils.waitFor(function() { return webView.lastGeolocationStatus != -1; }));
      compare(webView.lastGeolocationStatus, 0);

      spy.clear();
      webView.lastGeolocationStatus = -1;

      webView.url = data.url2;
      verify(webView.waitForLoadSucceeded());

      spy.wait();
    }

    function test_GeolocationPermissionRequest_session_persist4_data() {
      return [
        { url1: "https://testsuite/tst_GeolocationPermissionRequest_session_persist_embedder.html",
          url2: "https://testsuite/tst_GeolocationPermissionRequest_session_persist.html" },
        { url1: "https://testsuite/tst_GeolocationPermissionRequest_session_persist.html",
          url2: "https://testsuite/tst_GeolocationPermissionRequest_session_persist_embedder.html" },
      ];
    }

    // Verify cases where saved permissions should be used
    function test_GeolocationPermissionRequest_session_persist4(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = data.url1;
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      webView.lastGeolocationRequest.allow();

      verify(TestUtils.waitFor(function() { return webView.lastGeolocationStatus != -1; }));
      compare(webView.lastGeolocationStatus, 0);

      spy.clear();
      webView.lastGeolocationStatus = -1;

      webView.url = data.url2;
      verify(webView.waitForLoadSucceeded());

      verify(TestUtils.waitFor(function() { return webView.lastGeolocationStatus != -1; }));
      compare(webView.lastGeolocationStatus, 0);
    }
  }
}
