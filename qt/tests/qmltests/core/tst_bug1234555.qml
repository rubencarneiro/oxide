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
    name: "bug1234555"
    when: windowShown

    property var numberSent: 0
    property var errorsReceived: []

    function test_bug1234555() {
      webView.url = "http://testsuite/tst_bug1234555.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      function sendMessage(frame) {
        numberSent++;
        var req = frame.sendMessageNoWait("TEST-DONT-RESPOND");
        req.onerror = function(code, msg) {
          errorsReceived.push({code: code, msg: msg});
        };
      }

      // FIXME(chrisccoulson): Doesn't work for top-level navigations
      //  see https://launchpad.net/bugs/1287581
      //sendMessage(new ScriptMessageTestUtils.FrameHelper(webView.rootFrame));
      sendMessage(new ScriptMessageTestUtils.FrameHelper(webView.rootFrame.childFrames[0]));

      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      verify(TestUtils.waitFor(function() { return errorsReceived.length == numberSent; }),
             "Timed out waiting for responses");
      errorsReceived.forEach(function(error) {
        compare(error.code, ScriptMessageRequest.ErrorHandlerDidNotRespond);
      });
    }
  }
}
