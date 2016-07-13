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
    name: "bug1427828"
    when: windowShown

    function cleanupTestCase() {
      webView.context.clearTestUserScripts();
    }

    function test_bug1427828() {
      webView.url = "http://testsuite/tst_bug1427828.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      new ScriptMessageTestUtils.FrameHelper(
          webView.rootFrame.childFrames[0]).sendMessageNoReply("TEST-DONT-RESPOND");

      new ScriptMessageTestUtils.FrameHelper(
          webView.rootFrame).sendMessageNoReply("TEST-DONT-RESPOND");

      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }
  }
}
