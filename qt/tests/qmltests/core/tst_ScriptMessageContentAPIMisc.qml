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
    name: "ScriptMessageContentAPIMisc"
    when: windowShown

    function cleanupTestCase() {
      webView.context.clearTestUserScripts();
    }

    function init() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }

    function test_ScriptMessageContentAPIMisc1_ScriptMessage_id() {
      var res = new ScriptMessageTestUtils.FrameHelper(
          webView.rootFrame).sendMessage("TEST-REPLY", "msg.id");
      compare(res, "TEST-REPLY");
    }

    function test_ScriptMessageContentAPIMisc2_ScriptMessage_error_data() {
      return [ { payload: 10 },
               { payload: true },
               { payload: 1.65453543 },
               { payload: "This is a string" },
               { payload: undefined },
               { payload: [ "foo", 5, { a: 10, b: 5.7565, c: undefined }, false ] },
               { payload: { a: 7, b: "foo", c: [ 87.243532, true, undefined ] } }
      ];
    }

    function test_ScriptMessageContentAPIMisc2_ScriptMessage_error(data) {
      var res = new ScriptMessageTestUtils.FrameHelper(
          webView.rootFrame).sendMessage("TEST-ERROR", data.payload);

      verify(res instanceof TestUtils.MessageError);
      compare(res.error, ScriptMessageRequest.ErrorHandlerReportedError,
              "Unexpected error code");
      compare(res.message, data.payload);
    }
  }
}
