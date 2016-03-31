import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

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

  SignalSpy {
    id: requestSpy
    target: webView
    signalName: "fullscreenRequested"
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

    Connections {
      id: connection
      ignoreUnknownSignals: true

      onFullscreenRequested: {
        webView.fullscreen = fullscreen;
      }
    }

    function init() {
      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded());
      webView.fullscreen = false;
      spy.clear();
      requestSpy.clear();
      webView.fullscreenChangeEvents = 0;
      webView.clearLoadEventCounters();
    }

    // Test the defaults are as expected
    function test_WebView_fullscreen1_defaults() {
      webView.url = "http://testsuite/tst_WebView_fullscreen.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.fullscreen, false,
              "WebView.fullscreen should initially be false");
      compare(webView.getTestApi().evaluateCode("document.webkitFullscreenEnabled", false), true,
              "document.webkitFullscreenEnabled should be true");
    }

    // Test that granting fullscreen on a fullscreen request works
    function test_WebView_fullscreen2_grant_on_request() {
      webView.url = "http://testsuite/tst_WebView_fullscreen.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"document.addEventListener(\"webkitfullscreenchange\", function(event) {
  oxide.sendMessage(\"FULL-SCREEN-CHANGE\");
});", true);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      requestSpy.wait();

      compare(requestSpy.signalArguments[0][0], true, "Expected fullscreen == true");

      webView.fullscreen = true;

      compare(spy.count, 1, "Should have had 1 fullscreenChanged signal");
      compare(webView.fullscreen, true, "WebView.fullscreen should be true");

      verify(TestUtils.waitFor(function() { return fullscreenChangeEvents == 1; }),
             "Timed out waiting for webkitfullscreenchange event");
      compare(webView.getTestApi().evaluateCode("!!document.webkitFullscreenElement", false), true,
              "Fullscreen has been granted");
    }

    function test_WebView_fullscreen3_grant_before_request() {
      webView.url = "http://testsuite/tst_WebView_fullscreen.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"document.addEventListener(\"webkitfullscreenchange\", function(event) {
  oxide.sendMessage(\"FULL-SCREEN-CHANGE\");
});", true);

      webView.fullscreen = true;

      compare(spy.count, 1, "Should have had 1 fullscreenChanged signal");
      compare(webView.fullscreen, true, "WebView.fullscreen should be true");

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      verify(TestUtils.waitFor(function() { return fullscreenChangeEvents == 1; }),
             "Timed out waiting for webkitfullscreenchange event");
      compare(webView.getTestApi().evaluateCode("!!document.webkitFullscreenElement", false), true,
              "Fullscreen has been requested");
      compare(requestSpy.count, 0, "Shouldn't have had a request");
    }

    function test_WebView_fullscreen4_cancel_after_request() {
      webView.url = "http://testsuite/tst_WebView_fullscreen.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"document.addEventListener(\"webkitfullscreenchange\", function(event) {
  oxide.sendMessage(\"FULL-SCREEN-CHANGE\");
});", true);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      requestSpy.wait();

      webView.fullscreen = true;

      verify(TestUtils.waitFor(function() { return fullscreenChangeEvents == 1; }),
             "Timed out waiting for webkitfullscreenchange event");

      requestSpy.clear();
      spy.clear();
      webView.fullscreenChangeEvents = 0;

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      requestSpy.wait();

      compare(requestSpy.signalArguments[0][0], false, "Expected fullscreen == false");

      verify(TestUtils.waitFor(function() { return fullscreenChangeEvents == 1; }),
             "Timed out waiting for webkitfullscreenchange event");
      compare(webView.getTestApi().evaluateCode("!!document.webkitFullscreenElement", false), false,
              "Fullscreen has been cancelled");
    }

    function test_WebView_fullscreen5_cancel_before_request() {
      webView.url = "http://testsuite/tst_WebView_fullscreen.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"document.addEventListener(\"webkitfullscreenchange\", function(event) {
  oxide.sendMessage(\"FULL-SCREEN-CHANGE\");
});", true);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      requestSpy.wait();

      webView.fullscreen = true;

      verify(TestUtils.waitFor(function() { return fullscreenChangeEvents == 1; }),
             "Timed out waiting for webkitfullscreenchange event");

      requestSpy.clear();
      spy.clear();
      webView.fullscreenChangeEvents = 0;

      webView.fullscreen = false;

      verify(TestUtils.waitFor(function() { return fullscreenChangeEvents == 1; }),
             "Timed out waiting for webkitfullscreenchange event");
      compare(webView.getTestApi().evaluateCode("!!document.webkitFullscreenElement", false), false,
              "Fullscreen has been cancelled");
      compare(requestSpy.count, 0, "Shouldn't have had a request");
    }

    function test_WebView_fullscreen6_navigation_cancel() {
      webView.url = "http://testsuite/tst_WebView_fullscreen.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"document.addEventListener(\"webkitfullscreenchange\", function(event) {
  oxide.sendMessage(\"FULL-SCREEN-CHANGE\");
});", true);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      requestSpy.wait();

      webView.fullscreen = true;

      verify(TestUtils.waitFor(function() { return fullscreenChangeEvents == 1; }),
             "Timed out waiting for webkitfullscreenchange event");

      requestSpy.clear();
      spy.clear();
      webView.clearLoadEventCounters();
      webView.fullscreenChangeEvents = 0;

      webView.url = "about:blank";
      verify(webView.waitForLoadCommitted());
      requestSpy.wait();

      compare(requestSpy.signalArguments[0][0], false, "Expected fullscreen == false");

      verify(webView.waitForLoadSucceeded());
      // There's nothing else to check here, as the old document has gone away now
    }

    function test_WebView_fullscreen7_synchronous() {
      connection.target = webView;

      webView.url = "http://testsuite/tst_WebView_fullscreen.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.getTestApi().evaluateCode(
"document.addEventListener(\"webkitfullscreenchange\", function(event) {
  oxide.sendMessage(\"FULL-SCREEN-CHANGE\");
});", true);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      requestSpy.wait();

      compare(spy.count, 1, "Should have had 1 fullscreenChanged signal");
      compare(webView.fullscreen, true, "WebView.fullscreen should be true");

      verify(TestUtils.waitFor(function() { return fullscreenChangeEvents == 1; }),
             "Timed out waiting for webkitfullscreenchange event");
      compare(webView.getTestApi().evaluateCode("!!document.webkitFullscreenElement", false), true,
              "Fullscreen has been granted");

      requestSpy.clear();
      spy.clear();
      webView.fullscreenChangeEvents = 0;

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      requestSpy.wait();

      compare(spy.count, 1, "Should have had 1 fullscreenChanged signal");
      compare(webView.fullscreen, false, "WebView.fullscreen should be false");

      verify(TestUtils.waitFor(function() { return fullscreenChangeEvents == 1; }),
             "Timed out waiting for webkitfullscreenchange event");
      compare(webView.getTestApi().evaluateCode("!!document.webkitFullscreenElement", false), false,
              "Fullscreen has been cancelled");
    }
  }
}
