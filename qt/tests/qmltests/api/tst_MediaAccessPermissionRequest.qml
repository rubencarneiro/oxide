import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.8
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "mediaAccessPermissionRequested"
  }

  SignalSpy {
    id: cancelSpy
    signalName: "cancelled"
  }

  property var lastRequest: null
  onMediaAccessPermissionRequested: {
    lastRequest = request;
  }

  property string lastError: ""
  messageHandlers: [
    ScriptMessageHandler {
      msgId: "GUM-RESPONSE"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        webView.lastError = msg.args.error;
      }
    }
  ]

  TestCase {
    id: test
    name: "MediaAccessPermissionRequest"
    when: windowShown

    function _test_accept() {
      webView.lastRequest.allow();
    }

    function _test_deny() {
      webView.lastRequest.deny();
    }

    function _test_destroy() {
      webView.lastRequest.destroy();
    }

    function init() {
      webView.lastRequest = null;
      webView.lastError = "";
      spy.clear();
      cancelSpy.clear();
      cancelSpy.target = null;
      webView.clearLoadEventCounters();
    }

    function test_MediaAccessPermissionRequest1_properties_data() {
      return [
        { params: "audio=1&video=1", isForAudio: true, isForVideo: true },
        { params: "audio=1", isForAudio: true, isForVideo: false },
        { params: "video=1", isForAudio: false, isForVideo: true },
      ];
    }

    function test_MediaAccessPermissionRequest1_properties(data) {
      webView.url = "http://testsuite/tst_MediaAccessPermissionRequest.html?" + data.params;
      verify(webView.waitForLoadSucceeded());

      if (!webView.lastRequest) {
        spy.wait();
      }

      compare(webView.lastRequest.origin, "http://testsuite/");
      compare(webView.lastRequest.embedder, "http://testsuite/");
      compare(webView.lastRequest.isCancelled, false);
      compare(webView.lastRequest.isForAudio, data.isForAudio);
      compare(webView.lastRequest.isForVideo, data.isForVideo);
    }

    function test_MediaAccessPermissionRequest2_main_frame_data() {
      return [
        { function: _test_accept, expected: "OK" },
        { function: _test_deny, expected: "PermissionDeniedError" },
        { function: _test_destroy, expected: "PermissionDeniedError" },
      ];
    }

    function test_MediaAccessPermissionRequest2_main_frame(data) {
      webView.url = "http://testsuite/tst_MediaAccessPermissionRequest.html?video=1";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode(
"document.addEventListener(\"oxidegumresult\", function(event) {
  oxide.sendMessage(\"GUM-RESPONSE\", { error: event.detail.error });
});", true);

      if (!webView.lastRequest) {
        spy.wait();
      }

      compare(webView.lastRequest.origin, "http://testsuite/");
      compare(webView.lastRequest.embedder, "http://testsuite/");
      compare(webView.lastRequest.isCancelled, false);

      data.function();

      verify(webView.waitFor(function() { return webView.lastError != ""; }));
      compare(webView.lastError, data.expected);
    }

    function test_MediaAccessPermissionRequest3_subframe_data() {
      return [
        { function: _test_accept, expected: "OK" },
        { function: _test_deny, expected: "PermissionDeniedError" },
        { function: _test_destroy, expected: "PermissionDeniedError" },
      ];
    }

    function test_MediaAccessPermissionRequest3_subframe(data) {
      webView.url = "http://foo.testsuite/tst_MediaAccessPermissionRequest_subframe.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode(
"document.addEventListener(\"oxidegumresult\", function(event) {
  oxide.sendMessage(\"GUM-RESPONSE\", { error: event.detail.error });
});", true);

      if (!webView.lastRequest) {
        spy.wait();
      }

      compare(webView.lastRequest.origin, "http://testsuite/");
      compare(webView.lastRequest.embedder, "http://foo.testsuite/");
      compare(webView.lastRequest.isCancelled, false);

      data.function();

      verify(webView.waitFor(function() { return webView.lastError != ""; }));
      compare(webView.lastError, data.expected);
    }

    function test_MediaAccessPermissionRequest4_main_frame_navigation_cancel() {
      webView.url = "http://testsuite/tst_MediaAccessPermissionRequest.html?video=1";
      verify(webView.waitForLoadSucceeded());

      if (!webView.lastRequest) {
        spy.wait();
      }

      cancelSpy.target = webView.lastRequest;

      webView.clearLoadEventCounters();
      webView.getTestApi().evaluateCode(
          "window.location = \"http://testsuite/empty.html\";", false);
      verify(webView.waitForLoadCommitted());

      compare(cancelSpy.count, 1);
      verify(webView.lastRequest.isCancelled);
    }

    function test_MediaAccessPermissionRequest5_subframe_navigation_cancel() {
      webView.url = "http://testsuite/tst_MediaAccessPermissionRequest_subframe.html";
      verify(webView.waitForLoadSucceeded());

      if (!webView.lastRequest) {
        spy.wait();
      }

      cancelSpy.target = webView.lastRequest;

      webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode(
          "window.location = \"http://testsuite/empty.html\";", false);

      cancelSpy.wait();

      compare(cancelSpy.count, 1);
      verify(webView.lastRequest.isCancelled);
    }

    function test_MediaAccessPermissionRequest5_subframe_delete_cancel() {
      webView.url = "http://testsuite/tst_MediaAccessPermissionRequest_subframe.html";
      verify(webView.waitForLoadSucceeded());

      if (!webView.lastRequest) {
        spy.wait();
      }

      cancelSpy.target = webView.lastRequest;

      webView.getTestApi().evaluateCode("
var f = document.getElementsByTagName(\"iframe\")[0];
f.parentElement.removeChild(f);", true);

      cancelSpy.wait();

      compare(cancelSpy.count, 1);
      verify(webView.lastRequest.isCancelled);
    }
  }
}
