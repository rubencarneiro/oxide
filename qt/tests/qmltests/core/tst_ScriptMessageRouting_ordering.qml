import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property var receivedResponses: []
  property var numberSent: 0

  messageHandlers: [
    ScriptMessageHandler {
      msgId: "TEST"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        msg.reply({});
      }
    }
  ]

  TestCase {
    id: test
    name: "ScriptMessageRouting_ordering"
    when: windowShown

    function test_ScriptMessageRouting_ordering() {
      webView.url = "http://localhost:8080/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      function sendMessage() {
        var req = webView.rootFrame.sendMessage("oxide://testutils/", "SEND-MESSAGE-TO-SELF", { id: "TEST", args: {} });
        var serial = numberSent++;
        req.onreply = function(args) {
          webView.receivedResponses.push(serial);
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

      verify(webView.waitFor(function() { return webView.receivedResponses.length == numberSent; }),
             "Timed out waiting for responses");
    }
  }
}
