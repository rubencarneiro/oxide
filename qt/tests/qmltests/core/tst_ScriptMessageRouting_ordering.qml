import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  property var receivedResponses: 0
  property var numberSent: 0
  property var lastSerial: -1

  messageHandlers: [
    ScriptMessageTestHandler {
      msgId: "TEST"
      callback: function(msg) {
        msg.reply(msg.payload);
      }
    }
  ]

  Component.onCompleted: {
    ScriptMessageTestUtils.init(webView.context);
  }

  TestCase {
    id: test
    name: "ScriptMessageRouting_ordering"
    when: windowShown

    function cleanupTestCase() {
      webView.context.clearTestUserScripts();
    }

    function test_ScriptMessageRouting_ordering() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      function sendMessage() {
        var req = new ScriptMessageTestUtils.FrameHelper(webView.rootFrame).sendMessageToBrowserNoWait("TEST", webView.numberSent++);
        req.onreply = function(payload) {
          webView.receivedResponses++;
          verify(payload.response > webView.lastSerial);
          webView.lastSerial = payload.response;
        };
      }

      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();

      verify(TestUtils.waitFor(function() { return webView.receivedResponses == numberSent; }),
             "Timed out waiting for responses");
    }
  }
}
