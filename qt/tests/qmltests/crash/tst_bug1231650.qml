import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  Component.onCompleted: {
    ScriptMessageTestUtils.init(webView.context);
  }

  TestCase {
    id: test
    name: "bug1231650"
    when: windowShown

    property var requests: []

    function test_bug1231650() {
      webView.url = "http://testsuite/tst_bug1231650.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var api = new ScriptMessageTestUtils.FrameHelper(webView.rootFrame.childFrames[0]);

      function sendMessage() {
        requests.push(api.sendMessageNoWait("TEST-DONT-RESPOND"));
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

      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      while (requests.length > 0) {
        var req = requests.pop();
        TestSupport.destroyQObjectNow(req);
      }
    }
  }
}
