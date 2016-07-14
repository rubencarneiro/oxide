import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

Item {
  Component.onCompleted: {
    SingletonTestWebContext.addTestUserScript({
        context: "oxide://mediatest/",
        url: Qt.resolvedUrl("tst_MediaAccessPermissionRequest_session_persist.js"),
        incognitoEnabled: true,
        matchAllFrames: true
    });
  }

  SignalSpy {
    id: spy
    signalName: "mediaAccessPermissionRequested"
  }

  Component {
    id: webViewFactory
    TestWebView {
      property var lastRequest: null
      onMediaAccessPermissionRequested: {
        lastRequest = request;
      }

      QtObject {
        id: _internal
        property string lastError: ""
      }
      property alias lastError: _internal.lastError

      messageHandlers: [
        ScriptMessageHandler {
          msgId: "GUM-RESPONSE"
          contexts: [ "oxide://mediatest/" ]
          callback: function(msg) {
            _internal.lastError = msg.payload;
          }
        }
      ]
    }
  }

  TestCase {
    id: test
    name: "MediaAccessPermissionRequest_session_persist"
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

    function test_MediaAccessPermissionRequest_session_persist1_data() {
      return [
        { function: _test_accept, expected: "OK", save: true, params: "audio=1&video=1" },
        { function: _test_accept, expected: "OK", save: true, params: "audio=1" },
        { function: _test_accept, expected: "OK", save: true, params: "video=1" },
        { function: _test_deny, expected: "PermissionDeniedError", save: true, params: "audio=1&video=1" },
        { function: _test_deny, expected: "PermissionDeniedError", save: true, params: "audio=1" },
        { function: _test_deny, expected: "PermissionDeniedError", save: true, params: "video=1" },
        { function: _test_destroy, expected: "PermissionDeniedError", save: false, params: "audio=1&video=1" },
        { function: _test_destroy, expected: "PermissionDeniedError", save: false, params: "audio=1" },
        { function: _test_destroy, expected: "PermissionDeniedError", save: false, params: "video=1" },
      ];
    }

    // Verify that media permission request decisions are remembered by
    // responding to an initial request and then reloading the webview
    function test_MediaAccessPermissionRequest_session_persist1(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?" + data.params;
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      compare(webView.lastRequest.origin, "https://testsuite/");
      compare(webView.lastRequest.embedder, "https://foo.testsuite/");

      data.function(webView.lastRequest);

      verify(TestUtils.waitFor(function() { return webView.lastError != ""; }));
      compare(webView.lastError, data.expected);

      spy.clear();
      webView.lastError = "";

      webView.reload();
      verify(webView.waitForLoadSucceeded());

      if (data.save) {
        verify(TestUtils.waitFor(function() { return webView.lastError != ""; }));
        compare(webView.lastError, data.expected);
      } else {
        spy.wait();
        compare(webView.lastRequest.origin, "https://testsuite/");
        compare(webView.lastRequest.embedder, "https://foo.testsuite/");
      }
    }

    function test_MediaAccessPermissionRequest_session_persist2_data() {
      return test_MediaAccessPermissionRequest_session_persist1_data();
    }

    // Verify that media permission request decisions are remembered by
    // responding to an initial request in one webview, and then attempting the
    // same access in a new webview
    function test_MediaAccessPermissionRequest_session_persist2(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?" + data.params;
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      compare(webView.lastRequest.origin, "https://testsuite/");
      compare(webView.lastRequest.embedder, "https://foo.testsuite/");

      data.function(webView.lastRequest);

      verify(TestUtils.waitFor(function() { return webView.lastError != ""; }));
      compare(webView.lastError, data.expected);

      webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      spy.clear();

      webView.url = "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?" + data.params;
      verify(webView.waitForLoadSucceeded());

      if (data.save) {
        verify(TestUtils.waitFor(function() { return webView.lastError != ""; }));
        compare(webView.lastError, data.expected);
      } else {
        spy.wait();
        compare(webView.lastRequest.origin, "https://testsuite/");
        compare(webView.lastRequest.embedder, "https://foo.testsuite/");
      }
    }

    function test_MediaAccessPermissionRequest_session_persist3_data() {
      return [
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1",
          url2: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?audio=1" },
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?audio=1",
          url2: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1" },
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1",
          url2: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?audio=1&video=1" },
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?audio=1",
          url2: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?audio=1&video=1" },
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1",
          url2: "https://bar.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1" },
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?audio=1",
          url2: "https://testsuite/tst_MediaAccessPermissionRequest_session_persist.html?audio=1" },
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist.html?video=1&audio=1",
          url2: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1&audio=1" },
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist.html?video=1",
          url2: "https://bar.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1" },
      ];
    }

    // Verify several cases where saved permissions should not be used
    function test_MediaAccessPermissionRequest_session_persist3(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = data.url1;
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      webView.lastRequest.allow();

      verify(TestUtils.waitFor(function() { return webView.lastError != ""; }));
      compare(webView.lastError, "OK");

      spy.clear();
      webView.lastError = "";

      webView.url = data.url2;
      verify(webView.waitForLoadSucceeded());

      spy.wait();
    }

    function test_MediaAccessPermissionRequest_session_persist4_data() {
      return [
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1&audio=1",
          url2: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?audio=1",
          function: _test_accept, expected: "OK" },
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1&audio=1",
          url2: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1",
          function: _test_accept, expected: "OK" },
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1&audio=1",
          url2: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?audio=1",
          function: _test_deny, expected: "PermissionDeniedError" },
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1&audio=1",
          url2: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1",
          function: _test_deny, expected: "PermissionDeniedError" },
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?audio=1",
          url2: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?audio=1&video=1",
          function: _test_deny, expected: "PermissionDeniedError" },
        { url1: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1",
          url2: "https://foo.testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?audio=1&video=1",
          function: _test_deny, expected: "PermissionDeniedError" },
        { url1: "https://testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?video=1",
          url2: "https://testsuite/tst_MediaAccessPermissionRequest_session_persist.html?video=1",
          function: _test_accept, expected: "OK" },
        { url1: "https://testsuite/tst_MediaAccessPermissionRequest_session_persist.html?audio=1",
          url2: "https://testsuite/tst_MediaAccessPermissionRequest_session_persist_embedder.html?audio=1",
          function: _test_accept, expected: "OK" },
      ];
    }

    // Verify cases where saved permissions should be used
    function test_MediaAccessPermissionRequest_session_persist4(data) {
      var webView = webViewFactory.createObject(null, {});
      spy.target = webView;

      webView.url = data.url1;
      verify(webView.waitForLoadSucceeded());

      spy.wait();

      data.function(webView.lastRequest);

      verify(TestUtils.waitFor(function() { return webView.lastError != ""; }));
      compare(webView.lastError, data.expected);

      spy.clear();
      webView.lastError = "";

      webView.url = data.url2;
      verify(webView.waitForLoadSucceeded());

      verify(TestUtils.waitFor(function() { return webView.lastError != ""; }));
      compare(webView.lastError, data.expected);
    }
  }
}
