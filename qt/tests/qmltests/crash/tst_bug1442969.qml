import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property QtObject lastMessage: null

  messageHandlers: [
    ScriptMessageHandler {
      msgId: "TEST-ASYNC-REPLY"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        lastMessage = msg;
      }
    }
  ]

  SignalSpy {
    id: spy
    target: webView
    signalName: "frameRemoved"
  }

  TestCase {
    id: test
    name: "bug1442969"
    when: windowShown

    function test_bug1442969() {
      webView.url = "http://testsuite/tst_bug1442969.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      spy.clear();

      var res = null
      var req = webView.rootFrame.childFrames[0].sendMessage(
          "oxide://testutils/",
          "SEND-MESSAGE-TO-SELF", 
          { id: "TEST-ASYNC-REPLY",
            args: { in: 10 }});

      TestUtils.waitFor(function() { return !!webView.lastMessage; });
      compare(webView.lastMessage.frame, webView.rootFrame.childFrames[0]);

      webView.getTestApi().evaluateCode(
"var f = document.querySelector(\"iframe\");
f.parentNode.removeChild(f);", true);
      spy.wait();

      // Shouldn't crash
      verify(!webView.lastMessage.frame);
    }
  }
}
