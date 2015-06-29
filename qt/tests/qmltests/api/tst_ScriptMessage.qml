import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  property QtObject lastMessageFrameSource: null
  property QtObject lastMessage: null

  messageHandlers: [
    ScriptMessageTestHandler {
      msgId: "TEST-REPLY"
      callback: function(msg) {
        webView.lastMessageFrameSource = msg.frame;
        msg.reply({ out: msg.payload, id: msg.id, context: msg.context });
      }
    },
    ScriptMessageTestHandler {
      msgId: "TEST-ERROR"
      callback: function(msg) {
        webView.lastMessageFrameSource = msg.frame;
        msg.error(msg.payload);
      }
    },
    ScriptMessageTestHandler {
      msgId: "TEST-ASYNC-REPLY"
      callback: function(msg) {
        webView.lastMessage = msg;
      }
    }
  ]

  Component.onCompleted: {
    ScriptMessageTestUtils.init(webView.context);
  }

  // These tests verify that ScriptMessage works correctly - it also indirectly
  // tests ScriptMessageRequest.onreply and WebFrame.sendMessage, plus
  // ScriptMessage.payload / ScriptMessage.reply / ScriptMessageRequest.onreply /
  // ScriptMessageRequest.onerror / oxide.sendMessage on the content side
  TestCase {
    id: test
    name: "ScriptMessage"
    when: windowShown

    function init() {
      webView.lastMessageFrameSource = null;
      webView.lastMessage = null;
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }

    function _test_data() {
      // Whilst we can send null to the renderer side, the browser can't
      // receive null as it gets converted to undefined (there is no difference
      // between null or defined with QVariant).
      // We test undefined here, even though this is bogus JSON
      return [ { payload: 10 },
               { payload: true },
               { payload: 1.65453543 },
               { payload: "This is a string" },
               { payload: undefined },
               { payload: [ "foo", 5, { a: 10, b: 5.7565, c: undefined }, false ] },
               { payload: { a: 7, b: "foo", c: [ 87.243532, true, undefined ] } }
      ];
    }

    function test_ScriptMessage1_reply_data() { return _test_data(); }
    function test_ScriptMessage1_reply(data) {
      var api = new ScriptMessageTestUtils.FrameHelper(webView.rootFrame);
      var res = api.sendMessageToBrowser("TEST-REPLY", data.payload);
      compare(res.out, data.payload, "Invalid response from message handler");
      compare(webView.lastMessageFrameSource, webView.rootFrame,
              "Invalid source frame for message");
      compare(res.context, ScriptMessageTestUtils.kDefaultContextUrl,
              "Invalid context for message");
      compare(res.id, "TEST-REPLY", "Invalid ID for message");
    }

    function test_ScriptMessage2_error_data() { return _test_data(); }
    function test_ScriptMessage2_error(data) {
      var api = new ScriptMessageTestUtils.FrameHelper(webView.rootFrame);
      var res = api.sendMessageToBrowser("TEST-ERROR", data.payload);
      verify(res instanceof TestUtils.MessageError, "Invalid result type");
      compare(res.error, ScriptMessageRequest.ErrorHandlerReportedError,
              "Unexpected error type");
      compare(res.message, data.payload, "Unexpected error message");

      compare(webView.lastMessageFrameSource, webView.rootFrame,
              "Invalid source frame for message");
    }

    function test_ScriptMessage3_async_response() {
      var res = null
      var api = new ScriptMessageTestUtils.FrameHelper(webView.rootFrame);
      var req = api.sendMessageToBrowserNoWait("TEST-ASYNC-REPLY");
      req.onreply = function(r) {
        res = r.response;
      };

      TestUtils.waitFor(function() { return !!webView.lastMessage; });
      compare(webView.lastMessage.frame, webView.rootFrame);

      webView.lastMessage.reply(10);
      TestUtils.waitFor(function() { return !!res; });
      compare(res, 10);
    }
  }
}
