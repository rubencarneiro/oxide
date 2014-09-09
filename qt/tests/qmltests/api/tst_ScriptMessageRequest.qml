import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
  }


  TestCase {
    id: test
    name: "ScriptMessageRequest"
    when: windowShown

    function init() {
      webView.url = "http://localhost:8080/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }

    function test_ScriptMessageRequest1_onreply() {
      var req = webView.rootFrame.sendMessage("http://foo/", "FOO", {});
      spy.target = req;
      spy.signalName = "replyCallbackChanged";
      spy.clear();

      var func = function() {};

      req.onreply = func;
      compare(spy.count, 1, "Should have had a signal");
      compare(req.onreply, func, "Unexpected handler");

      req.onreply = req.onreply;
      compare(spy.count, 1, "Shouldn't have had a signal");

      req.onreply = "foo";
      compare(spy.count, 1, "Shouldn't have had a signal");
      compare(req.onreply, func, "Handler should be unchanged");

      req.onreply = null;
      compare(spy.count, 2, "Should have had a signal");
      compare(req.onreply, null, "Unexpected handler");
    }

    function test_ScriptMessageRequest2_onerror() {
      var req = webView.rootFrame.sendMessage("http://foo/", "FOO", {});
      spy.target = req;
      spy.signalName = "errorCallbackChanged";
      spy.clear();

      var func = function() {};

      req.onerror = func;
      compare(spy.count, 1, "Should have had a signal");
      compare(req.onerror, func, "Unexpected handler");

      req.onerror = req.onerror;
      compare(spy.count, 1, "Shouldn't have had a signal");

      req.onerror = "foo";
      compare(spy.count, 1, "Shouldn't have had a signal");
      compare(req.onerror, func, "Handler should be unchanged");

      req.onerror = null;
      compare(spy.count, 2, "Should have had a signal");
      compare(req.onerror, null, "Unexpected handler");
    }

    function test_ScriptMessageRequest3_invalid_dest() {
      var req = webView.rootFrame.sendMessage("http://foo/", "GET-DOCUMENT-URI", {});
      var hasError = false;
      var errorCode;
      req.onerror = function(code, msg) {
        hasError = true;
        errorCode = code;
      };

      verify(webView.waitFor(function() { return hasError; }),
             "Timed out waiting for error");
      compare(errorCode, ScriptMessageRequest.ErrorDestinationNotFound,
              "Unexpected error code");
    }

    function test_ScriptMessageRequest4_handler_throws() {
      var req = webView.rootFrame.sendMessage("oxide://testutils/", "GENERATE-JS-EXCEPTION", {});
      var hasError = false;
      var errorCode;
      req.onerror = function(code, msg) {
        hasError = true;
        errorCode = code;
      };

      verify(webView.waitFor(function() { return hasError; }),
             "Timed out waiting for error");
      compare(errorCode, ScriptMessageRequest.ErrorUncaughtException,
              "Unexpected error code");
    }

    function test_ScriptMessageRequest5_no_handler() {
      var req = webView.rootFrame.sendMessage("oxide://testutils/", "BLAAAAAAAAAAAAAH", {});
      var hasError = false;
      var errorCode;
      req.onerror = function(code, msg) {
        hasError = true;
        errorCode = code;
      };

      verify(webView.waitFor(function() { return hasError; }),
             "Timed out waiting for error");
      compare(errorCode, ScriptMessageRequest.ErrorNoHandler,
              "Unexpected error code");
    }

    function test_ScriptMessageRequest6_handler_report_error() {
      var req = webView.rootFrame.sendMessage("oxide://testutils/", "EVALUATE-CODE", { code: "foo" });
      var hasError = false;
      var errorCode;
      req.onerror = function(code, msg) {
        hasError = true;
        errorCode = code;
      };

      verify(webView.waitFor(function() { return hasError; }),
             "Timed out waiting for error");
      compare(errorCode, ScriptMessageRequest.ErrorHandlerReportedError,
              "Unexpected error code");
    }

    function test_ScriptMessageRequest7_handler_no_response() {
      skip("Currently times out because the renderer side object is never collected");
      return;

      var req = webView.rootFrame.sendMessage("oxide://testutils/", "DONT-RESPOND", {});
      var hasError = false;
      var errorCode;
      req.onerror = function(code, msg) {
        hasError = true;
        errorCode = code;
      };

      verify(webView.waitFor(function() { return hasError; }),
             "Timed out waiting for error");
      compare(errorCode, ScriptMessageRequest.ErrorHandlerDidNotRespond,
              "Unexpected error code");
    }
  }
}
