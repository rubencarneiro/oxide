import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property QtObject lastMessage: null

  Component.onCompleted: {
    ScriptMessageTestUtils.init(webView.context);
  }

  messageHandlers: [
    ScriptMessageTestHandler {
      msgId: "TEST-ASYNC-REPLY"
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

      var req = new ScriptMessageTestUtils.FrameHelper(
          webView.rootFrame.childFrames[0]).sendMessageToBrowserNoWait(
            "TEST-ASYNC-REPLY");

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
