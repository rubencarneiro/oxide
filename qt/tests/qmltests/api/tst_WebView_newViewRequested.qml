import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

Item {
  id: toplevel

  Component {
    id: webViewFactory
    TestWebView {
      context: WebContext {}
      url: "about:blank"
      incognito: true
    }
  }

  property var created: null

  TestWebView {
    id: webView1
    anchors.fill: parent

    onNewViewRequested: {
      created = webViewFactory.createObject(toplevel, { request: request });
    }
  }

  TestWebView {
    id: webView2
    anchors.fill: parent
  }

  SignalSpy {
    id: spy
    target: webView1
    signalName: "newViewRequested"
  }

  SignalSpy {
    id: navigationSpy
    target: webView1
    signalName: "navigationRequested"
  }

  TestCase {
    id: test
    name: "WebView_newViewRequested"
    when: windowShown

    function init() {
      if (created) {
        created.destroy();
        created = null;
      }
      spy.clear();
      navigationSpy.clear();
      webView1.z = 0;
      webView2.z = 0;
    }

    // Test that WebView.newViewRequested is emitted when window.open() is called
    function test_WebView_newViewRequested1_correct() {
      webView1.z = 1;
      navigationSpy.target = webView1;

      webView1.url = "http://testsuite/tst_WebView_newViewRequested.html";
      verify(webView1.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView1.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView1, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      spy.wait();
      verify(created.waitForLoadCommitted());

      compare(navigationSpy.count, 1, "Should have had an onNavigationRequested");
      compare(created.url, "http://testsuite/empty.html", "Unexpected URL");
      compare(created.context, webView1.context, "Unexpected context");
      compare(created.incognito, webView1.incognito, "WebView.incognito should match opener");

      TestUtils.waitFor(function() { return created.loading == false; });
      compare(created.getTestApi().evaluateCode("return window.opener.document.domain;", true), "testsuite");
    }

    // Test that a top-level navigation occurs when window.open() is called and
    // there are no handlers for WebView.newViewRequested
    function test_WebView_newViewRequested3_no_handler() {
      webView2.z = 1;
      navigationSpy.target = webView2;

      webView2.url = "http://testsuite/tst_WebView_newViewRequested.html";
      verify(webView2.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView2.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView2, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      verify(webView2.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      // See https://launchpad.net/bugs/1302740
      compare(navigationSpy.count, 1, "Should have had an onNavigationRequested");
      compare(webView2.url, "http://testsuite/empty.html", "Unexpected URL");
    }

    // Test that WebView.newViewRequested is emitted for non CurrentTab navigations
    // (clicking on a link with keyboard modifiers pressed)
    function test_WebView_newViewRequested4_from_navigation() {
      webView1.z = 1;
      navigationSpy.target = webView1;

      webView1.url = "http://testsuite/tst_WebView_newViewRequested2.html";
      verify(webView1.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      navigationSpy.clear();

      var r = webView1.getTestApi().getBoundingClientRectForSelector("#link");
      mouseClick(webView1, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, Qt.ShiftModifier);

      spy.wait();

      compare(navigationSpy.count, 1, "Should have had an onNavigationRequested");
      compare(created.url, "http://testsuite/empty.html", "Unexpected URL");
      compare(created.context, webView1.context, "Unexpected context");
      compare(created.incognito, webView1.incognito, "WebView.incognito should match opener");

      verify(TestUtils.waitFor(function() { return !created.loading; }));

      // See https://launchpad.net/bugs/1301004
      skip(created.getTestApi().evaluateCode("return window.opener != null;", true));
    }

    function test_WebView_newViewRequested5_no_handler_from_navigation() {
      webView2.z = 1;
      navigationSpy.target = webView2;

      webView2.url = "http://testsuite/tst_WebView_newViewRequested2.html";
      verify(webView2.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView2.getTestApi().getBoundingClientRectForSelector("#link");
      mouseClick(webView2, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, Qt.ShiftModifier);

      verify(webView2.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compare(navigationSpy.count, 1, "Should have had an onNavigationRequested");
      compare(webView2.url, "http://testsuite/empty.html", "Unexpected URL");
    }

    // Test that dynamically attaching a handler for WebView.newViewRequested works
    function test_WebView_newViewRequested6_dynamic() {
      webView2.z = 1;
      webView2.url = "http://testsuite/tst_WebView_newViewRequested3.html";
      verify(webView2.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView2.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView2, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      verify(webView2.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compare(webView2.url, "http://testsuite/tst_WebView_newViewRequested3.html?1");

      spy.target = webView2;
      var handler = function(request) {
        created = webViewFactory.createObject(toplevel, { request: request });
      };
      webView2.newViewRequested.connect(handler);

      var r = webView2.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView2, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      spy.wait();
      verify(created.waitForLoadCommitted());

      compare(spy.count, 1);
      compare(created.url, "http://testsuite/tst_WebView_newViewRequested3.html?2");
      compare(webView2.url, "http://testsuite/tst_WebView_newViewRequested3.html?1");

      spy.target = null;
      webView2.newViewRequested.disconnect(handler);

      var r = webView2.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView2, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      verify(webView2.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compare(webView2.url, "http://testsuite/tst_WebView_newViewRequested3.html?2");
    }
  }
}
