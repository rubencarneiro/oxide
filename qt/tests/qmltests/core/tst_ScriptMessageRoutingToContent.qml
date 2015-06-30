import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  Component.onCompleted: {
    ScriptMessageTestUtils.init(webView.context);
  }

  TestCase {
    id: test
    name: "ScriptMessageRoutingToContent"
    when: windowShown

    function test_ScriptMessageRoutingToContent() {
      webView.url = "http://testsuite/tst_ScriptMessageRoutingToContent.html";
      verify(webView.waitForLoadSucceeded());

      var res = new ScriptMessageTestUtils.FrameHelper(
          webView.rootFrame).sendMessage("TEST-REPLY", "document.documentURI");
      compare(res, webView.rootFrame.url);

      var res = new ScriptMessageTestUtils.FrameHelper(
          webView.rootFrame.childFrames[0]).sendMessage("TEST-REPLY", "document.documentURI");
      compare(res, webView.rootFrame.childFrames[0].url);
    } 
  }
}
