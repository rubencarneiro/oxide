import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property QtObject lastMessageFrameSource: null
  property QtObject lastMessage: null

  messageHandlers: [
    ScriptMessageHandler {
      msgId: "TEST-REPLY"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        webView.lastMessageFrameSource = msg.frame;
        msg.reply({ out: msg.payload, id: msg.id, context: msg.context });
      }
    },
    ScriptMessageHandler {
      msgId: "TEST-ERROR"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        webView.lastMessageFrameSource = msg.frame;
        msg.error("This is an error");
      }
    },
    ScriptMessageHandler {
      msgId: "TEST-NO-RESPONSE"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        webView.lastMessageFrameSource = msg.frame;
      }
    },
    ScriptMessageHandler {
      msgId: "TEST-ASYNC-REPLY"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        lastMessage = msg;
      }
    }
  ]

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

    function test_ScriptMessage1_reply() {
      var res = webView.getTestApi().sendMessageToSelf("TEST-REPLY", 10);
      compare(res.out, 10, "Invalid response from message handler");
      compare(webView.lastMessageFrameSource, webView.rootFrame,
              "Invalid source frame for message");
      compare(res.context, "oxide://testutils/", "Invalid context for message");
      compare(res.id, "TEST-REPLY", "Invalid ID for message");
    }

    function test_ScriptMessage2_error() {
      try {
        webView.getTestApi().sendMessageToSelf("TEST-ERROR");
        fail("Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, ScriptMessageRequest.ErrorHandlerReportedError,
                "Unexpected error type");
        compare(e.message, "This is an error", "Unexpected error message");
      }

      compare(webView.lastMessageFrameSource, webView.rootFrame,
              "Invalid source frame for message");
    }

    function test_ScriptMessage3_no_response() {
      try {
        webView.getTestApi().sendMessageToSelf("TEST-NO-RESPONSE", {}, true);
        fail("Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, ScriptMessageRequest.ErrorHandlerDidNotRespond,
                "Unexpected error type");
      }

      compare(webView.lastMessageFrameSource, webView.rootFrame,
              "Invalid source frame for message");
    }

    function test_ScriptMessage4_async_response() {
      var res = null
      var req = webView.rootFrame.sendMessage("oxide://testutils/",
                                              "SEND-MESSAGE-TO-SELF", 
                                              { id: "TEST-ASYNC-REPLY",
                                                args: { in: 10 }});
      req.onreply = function(r) {
        res = r.response;
      };

      webView.waitFor(function() { return !!webView.lastMessage; });
      compare(webView.lastMessage.frame, webView.rootFrame);

      webView.lastMessage.reply({ out: 10 });
      webView.waitFor(function() { return !!res; });
      compare(res.out, 10);
    }
  }
}
