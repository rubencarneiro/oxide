import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  property var lastGeolocationStatus: -1

  messageHandlers: [
    ScriptMessageHandler {
      msgId: "GEOLOCATION-RESPONSE"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        webView.lastGeolocationStatus = msg.payload;
      }
    }
  ]

  TestCase {
    id: test
    name: "bug1361868"
    when: windowShown

    function test_bug1361868_1() {
      webView.url = "http://testsuite/tst_bug1361868.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"document.addEventListener(\"oxidegeolocationresult\", function(event) {
  oxide.sendMessage(\"GEOLOCATION-RESPONSE\", event.detail.status);
});", true);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      verify(TestUtils.waitFor(
                 function() { return webView.lastGeolocationStatus != -1; },
                 null, true),
             "Timed out waiting for a response");
      compare(webView.lastGeolocationStatus, 1);
    }
  }
}
