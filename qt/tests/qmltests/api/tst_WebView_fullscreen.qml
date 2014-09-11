import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  property var fullscreenChangeEvents: 0

  messageHandlers: [
    ScriptMessageHandler {
      msgId: "FULL-SCREEN-CHANGE"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        fullscreenChangeEvents++;
      }
    }
  ]

  onFullscreenRequested: {
    webView.fullscreen = fullscreen;
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "fullscreenChanged"
  }

  TestCase {
    id: test
    name: "WebView_fullscreen"
    when: windowShown

    function test_WebView_fullscreen1() {
      webView.url = "http://testsuite/tst_WebView_fullscreen.html"
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"document.addEventListener(\"webkitfullscreenchange\", function(event) {
  oxide.sendMessage(\"FULL-SCREEN-CHANGE\");
});", true);

      compare(webView.fullscreen, false,
              "WebView.fullscreen should initially be false");
      compare(webView.getTestApi().evaluateCode("document.webkitFullscreenEnabled", false), true,
              "document.webkitFullscreenEnabled should be true");

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      spy.wait();

      compare(spy.count, 1, "Should have had 1 fullscreenChanged signal");
      compare(webView.fullscreen, true, "WebView.fullscreen should be true");

      verify(webView.waitFor(function() { return fullscreenChangeEvents == 1; }),
             "Timed out waiting for webkitfullscreenchange event");

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      spy.wait();

      compare(spy.count, 2, "Should have had another fullscreenChanged signal");
      compare(webView.fullscreen, false, "WebView.fullscreen should be false");

      verify(webView.waitFor(function() { return fullscreenChangeEvents == 2; }),
             "Timed out waiting for webkitfullscreenchange event");
    }
  }
}
