import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  Component.onCompleted: {
    ScriptMessageTestUtils.init(webView.context);
  }

  TestCase {
    id: test
    name: "ScriptMessageRoutingToContent_errors"
    when: windowShown

    function cleanupTestCase() {
      webView.context.clearTestUserScripts();
    }

    function init() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }

    function test_ScriptMessageRoutingToContent_errors1_invalid_dest() {
      var res = new ScriptMessageTestUtils.FrameHelper(
          webView.rootFrame, "http://foo/")
          .sendMessage("TEST-GENERATE-JS-EXCEPTION");

      verify(res instanceof TestUtils.MessageError);
      compare(res.error, ScriptMessageRequest.ErrorDestinationNotFound,
              "Unexpected error code");
      compare(res.message, undefined);
    }

    function test_ScriptMessageRoutingToContent_errors2_handler_throws() {
      var res = new ScriptMessageTestUtils.FrameHelper(
          webView.rootFrame).sendMessage("TEST-GENERATE-JS-EXCEPTION");

      verify(res instanceof TestUtils.MessageError);
      compare(res.error, ScriptMessageRequest.ErrorUncaughtException,
              "Unexpected error code");
      compare(res.message, "Uncaught Error: This is an exception");
    }

    function test_ScriptMessageRoutingToContent_errors3_no_handler() {
      var res = new ScriptMessageTestUtils.FrameHelper(
          webView.rootFrame).sendMessage("BLAAAAAAAAAH");

      verify(res instanceof TestUtils.MessageError);
      compare(res.error, ScriptMessageRequest.ErrorNoHandler,
              "Unexpected error code");
      compare(res.message, undefined);
    }

    function test_ScriptMessageRoutingToContent_errors4_handler_no_response() {
      skip("See https://launchpad.net/bugs/1468473");
      return;

      var res = new ScriptMessageTestUtils.FrameHelper(
          webView.rootFrame).sendMessage("TEST-DONT-RESPOND");

      verify(res instanceof TestUtils.MessageError);
      compare(res.error, ScriptMessageRequest.ErrorHandlerDidNotRespond,
              "Unexpected error code");
      compare(res.message, undefined);
    }
  }
}
