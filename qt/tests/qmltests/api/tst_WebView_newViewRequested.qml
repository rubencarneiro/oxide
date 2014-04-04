import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

Column {
  id: column
  focus: true

  Component {
    id: webViewFactory
    TestWebView {
      context: WebContext {}
      url: "about:blank"
      incognito: true
    }
  }

  property var created: null

  TestWebContext {
    id: context
  }

  TestWebView {
    id: webView1
    width: 200
    height: 200
    context: context

    onNewViewRequested: {
      created = webViewFactory.createObject(column, { request: request, width: 200, height: 50 });
    }
  }

  TestWebView {
    id: webView2
    width: 200
    height: 200
    context: context

    onNewViewRequested: {
      created = webViewFactory.createObject(column, { width: 200, height: 50 });
    }
  }

  TestWebView {
    id: webView3
    width: 200
    height: 200
    context: context
  }

  SignalSpy {
    id: spy
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
    }

    function test_WebView_newViewRequested1_correct() {
      spy.target = webView1;
      webView1.url = "http://localhost:8080/tst_WebView_newViewRequested.html";
      verify(webView1.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView1.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView1, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      spy.wait();

      compare(created.url, "http://localhost:8080/empty.html", "Unexpected URL");
      compare(created.context, webView1.context, "Unexpected context");
      compare(created.incognito, webView1.incognito, "WebView.incognito should match opener");

      webView1.waitFor(function() { return created.loading == false; });
      compare(created.getTestApi().evaluateCode("return window.opener.document.domain;", true), "localhost");
    }

    function test_WebView_newViewRequested2_incorrect() {
      spy.target = webView2;
      webView2.url = "http://localhost:8080/tst_WebView_newViewRequested.html";
      verify(webView2.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView2.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView2, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      spy.wait();

      compare(created.url, "about:blank", "Unexpected URL");
      verify(created.context != webView2.context, "Unexpected context");
      compare(created.incognito, true);
      try {
        verify(created.getTestApi().evaluateCode("return window.opener != null;", true));
        fail("Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError);
        compare(e.error, ScriptMessageRequest.ErrorDestinationNotFound);
      }
    }

    function test_WebView_newViewRequested3_no_handler() {
      webView3.url = "http://localhost:8080/tst_WebView_newViewRequested.html";
      verify(webView3.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView3.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView3, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      verify(webView3.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compare(webView3.url, "http://localhost:8080/empty.html", "Unexpected URL");
    }

    function test_WebView_newViewRequested4_from_navigation() {
      spy.target = webView1;
      webView1.url = "http://localhost:8080/tst_WebView_newViewRequested2.html";
      verify(webView1.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      navigationSpy.clear();

      var r = webView1.getTestApi().getBoundingClientRectForSelector("#link");
      mouseClick(webView1, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton, Qt.ShiftModifier);

      spy.wait();

      compare(navigationSpy.count, 1, "Should have had an onNavigationRequested");
      compare(created.url, "http://localhost:8080/empty.html", "Unexpected URL");
      compare(created.context, webView1.context, "Unexpected context");
      compare(created.incognito, webView1.incognito, "WebView.incognito should match opener");
      //verify(created.getTestApi().evaluateCode("return window.opener != null;", true));
    }
  }
}
