import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.19
import Oxide.testsupport 1.0

Item {
  id: top
  focus: true

  SignalSpy {
    id: spy
    signalName: "changed"
  }

  property var webView: null

  Component {
    id: webViewComponent
    TestWebView {}
  }

  TestCase {
    id: test
    name: "NavigationHistory"
    when: windowShown

    function init() {
      webView = webViewComponent.createObject(top, { "anchors.fill": parent });
      spy.target = webView.navigationHistory;
      spy.clear();
    }

    function cleanup() {
      TestSupport.destroyQObjectNow(webView);
    }

    function loadUrl(url) {
      webView.url = url;
      verify(webView.waitForLoadSucceeded(), url);
    }

    function verifyItems(prop, items, msg) {
      var length = webView.navigationHistory[prop].length;
      compare(length, items.length, msg);

      for (var i = 0; i < length; ++i) {
        compare(webView.navigationHistory[prop][i].url, items[i].url, msg);
        compare(webView.navigationHistory[prop][i].originalUrl, items[i].originalUrl, msg);
        compare(webView.navigationHistory[prop][i].title, items[i].title, msg);
      }
    }

    function test_NavigationHistory1_initial() {
      compare(webView.navigationHistory.backItems.length, 0);
      compare(webView.navigationHistory.forwardItems.length, 0);
      compare(webView.navigationHistory.items.length, 0);
      compare(webView.navigationHistory.currentItem, null);
      compare(webView.navigationHistory.currentItemIndex, -1);
      verify(!webView.navigationHistory.canGoBack);
      verify(!webView.navigationHistory.canGoForward);
    }

    function test_NavigationHistory2_backItems() {
      verifyItems("backItems", [], "Test start");

      loadUrl("http://testsuite/tst_NavigationHistory1.html");

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory2.html");

      verify(spy.count > 1);
      verifyItems("backItems", [
          { url: "http://testsuite/tst_NavigationHistory1.html", originalUrl: "http://testsuite/tst_NavigationHistory1.html", title: "Navigation test 1" }
      ], "Step 1");

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory3.html");

      verify(spy.count > 1);
      verifyItems("backItems", [
          { url: "http://testsuite/tst_NavigationHistory2.html", originalUrl: "http://testsuite/tst_NavigationHistory2.html", title: "Navigation test 2" },
          { url: "http://testsuite/tst_NavigationHistory1.html", originalUrl: "http://testsuite/tst_NavigationHistory1.html", title: "Navigation test 1" }
      ], "Step 2");

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory4.html");

      verify(spy.count > 1);
      verifyItems("backItems", [
          { url: "http://testsuite/tst_NavigationHistory3.html", originalUrl: "http://testsuite/tst_NavigationHistory3.html", title: "" },
          { url: "http://testsuite/tst_NavigationHistory2.html", originalUrl: "http://testsuite/tst_NavigationHistory2.html", title: "Navigation test 2" },
          { url: "http://testsuite/tst_NavigationHistory1.html", originalUrl: "http://testsuite/tst_NavigationHistory1.html", title: "Navigation test 1" }
      ], "Step 3");

      spy.clear();
      webView.navigationHistory.goBack();
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 1);
      verifyItems("backItems", [
          { url: "http://testsuite/tst_NavigationHistory2.html", originalUrl: "http://testsuite/tst_NavigationHistory2.html", title: "Navigation test 2" },
          { url: "http://testsuite/tst_NavigationHistory1.html", originalUrl: "http://testsuite/tst_NavigationHistory1.html", title: "Navigation test 1" }
      ], "Step 4");

      spy.clear();
      webView.navigationHistory.goBack();
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 1);
      verifyItems("backItems", [
          { url: "http://testsuite/tst_NavigationHistory1.html", originalUrl: "http://testsuite/tst_NavigationHistory1.html", title: "Navigation test 1" }
      ], "Step 5");

      spy.clear();
      webView.navigationHistory.goBack();
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 1);
      verifyItems("backItems", [], "Step 6");
    }

    function test_NavigationHistory3_forwardItems() {
      verifyItems("forwardItems", [], "Test start");

      loadUrl("http://testsuite/tst_NavigationHistory1.html");
      loadUrl("http://testsuite/tst_NavigationHistory2.html");
      loadUrl("http://testsuite/tst_NavigationHistory3.html");
      loadUrl("http://testsuite/tst_NavigationHistory4.html");

      verifyItems("forwardItems", [], "After populating history");

      spy.clear();
      webView.navigationHistory.goBack();
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 1);
      verifyItems("forwardItems", [
          { url: "http://testsuite/tst_NavigationHistory4.html", originalUrl: "http://testsuite/tst_NavigationHistory4.html", title: "Navigation test 4" }
      ], "Step 1");

      spy.clear();
      webView.navigationHistory.goBack();
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 1);
      verifyItems("forwardItems", [
          { url: "http://testsuite/tst_NavigationHistory3.html", originalUrl: "http://testsuite/tst_NavigationHistory3.html", title: "" },
          { url: "http://testsuite/tst_NavigationHistory4.html", originalUrl: "http://testsuite/tst_NavigationHistory4.html", title: "Navigation test 4" }
      ], "Step 2");

      spy.clear();
      webView.navigationHistory.goBack();
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 1);
      verifyItems("forwardItems", [
          { url: "http://testsuite/tst_NavigationHistory2.html", originalUrl: "http://testsuite/tst_NavigationHistory2.html", title: "Navigation test 2" },
          { url: "http://testsuite/tst_NavigationHistory3.html", originalUrl: "http://testsuite/tst_NavigationHistory3.html", title: "" },
          { url: "http://testsuite/tst_NavigationHistory4.html", originalUrl: "http://testsuite/tst_NavigationHistory4.html", title: "Navigation test 4" }
      ], "Step 3");

      spy.clear();
      webView.navigationHistory.goForward();
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 1);
      verifyItems("forwardItems", [
          { url: "http://testsuite/tst_NavigationHistory3.html", originalUrl: "http://testsuite/tst_NavigationHistory3.html", title: "" },
          { url: "http://testsuite/tst_NavigationHistory4.html", originalUrl: "http://testsuite/tst_NavigationHistory4.html", title: "Navigation test 4" }
      ], "Step 4");

      spy.clear();
      webView.navigationHistory.goForward();
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 1);
      verifyItems("forwardItems", [
          { url: "http://testsuite/tst_NavigationHistory4.html", originalUrl: "http://testsuite/tst_NavigationHistory4.html", title: "Navigation test 4" }
      ], "Step 5");

      spy.clear();
      webView.navigationHistory.goForward();
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 1);
      verifyItems("forwardItems", [], "Step 6");

      while (webView.navigationHistory.canGoBack) {
        webView.navigationHistory.goBack();
        verify(webView.waitForLoadSucceeded());
      }

      verifyItems("forwardItems", [
          { url: "http://testsuite/tst_NavigationHistory2.html", originalUrl: "http://testsuite/tst_NavigationHistory2.html", title: "Navigation test 2" },
          { url: "http://testsuite/tst_NavigationHistory3.html", originalUrl: "http://testsuite/tst_NavigationHistory3.html", title: "" },
          { url: "http://testsuite/tst_NavigationHistory4.html", originalUrl: "http://testsuite/tst_NavigationHistory4.html", title: "Navigation test 4" }
      ], "Step 7");

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory4.html");

      verify(spy.count > 1);
      verifyItems("forwardItems", [], "Step 8");
    }

    function test_NavigationHistory4_items() {
      verifyItems("items", [], "Test start");

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory1.html");

      verify(spy.count > 1);
      verifyItems("items", [
          { url: "http://testsuite/tst_NavigationHistory1.html", originalUrl: "http://testsuite/tst_NavigationHistory1.html", title: "Navigation test 1" }
      ], "Step 1");

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory2.html");

      verify(spy.count > 1);
      verifyItems("items", [
          { url: "http://testsuite/tst_NavigationHistory1.html", originalUrl: "http://testsuite/tst_NavigationHistory1.html", title: "Navigation test 1" },
          { url: "http://testsuite/tst_NavigationHistory2.html", originalUrl: "http://testsuite/tst_NavigationHistory2.html", title: "Navigation test 2" }
      ], "Step 2");

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory3.html");

      verify(spy.count > 1);
      verifyItems("items", [
          { url: "http://testsuite/tst_NavigationHistory1.html", originalUrl: "http://testsuite/tst_NavigationHistory1.html", title: "Navigation test 1" },
          { url: "http://testsuite/tst_NavigationHistory2.html", originalUrl: "http://testsuite/tst_NavigationHistory2.html", title: "Navigation test 2" },
          { url: "http://testsuite/tst_NavigationHistory3.html", originalUrl: "http://testsuite/tst_NavigationHistory3.html", title: "" }
      ], "Step 3");

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory4.html");

      verify(spy.count > 1);
      verifyItems("items", [
          { url: "http://testsuite/tst_NavigationHistory1.html", originalUrl: "http://testsuite/tst_NavigationHistory1.html", title: "Navigation test 1" },
          { url: "http://testsuite/tst_NavigationHistory2.html", originalUrl: "http://testsuite/tst_NavigationHistory2.html", title: "Navigation test 2" },
          { url: "http://testsuite/tst_NavigationHistory3.html", originalUrl: "http://testsuite/tst_NavigationHistory3.html", title: "" },
          { url: "http://testsuite/tst_NavigationHistory4.html", originalUrl: "http://testsuite/tst_NavigationHistory4.html", title: "Navigation test 4" }
      ], "Step 4");

      spy.clear();
      webView.navigationHistory.goBack();
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 1);
      verifyItems("items", [
          { url: "http://testsuite/tst_NavigationHistory1.html", originalUrl: "http://testsuite/tst_NavigationHistory1.html", title: "Navigation test 1" },
          { url: "http://testsuite/tst_NavigationHistory2.html", originalUrl: "http://testsuite/tst_NavigationHistory2.html", title: "Navigation test 2" },
          { url: "http://testsuite/tst_NavigationHistory3.html", originalUrl: "http://testsuite/tst_NavigationHistory3.html", title: "" },
          { url: "http://testsuite/tst_NavigationHistory4.html", originalUrl: "http://testsuite/tst_NavigationHistory4.html", title: "Navigation test 4" }
      ], "Step 5");

      while (webView.navigationHistory.canGoBack) {
        webView.navigationHistory.goBack();
        verify(webView.waitForLoadSucceeded());
      }

      verifyItems("items", [
          { url: "http://testsuite/tst_NavigationHistory1.html", originalUrl: "http://testsuite/tst_NavigationHistory1.html", title: "Navigation test 1" },
          { url: "http://testsuite/tst_NavigationHistory2.html", originalUrl: "http://testsuite/tst_NavigationHistory2.html", title: "Navigation test 2" },
          { url: "http://testsuite/tst_NavigationHistory3.html", originalUrl: "http://testsuite/tst_NavigationHistory3.html", title: "" },
          { url: "http://testsuite/tst_NavigationHistory4.html", originalUrl: "http://testsuite/tst_NavigationHistory4.html", title: "Navigation test 4" }
      ], "Step 6");

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory4.html");

      verify(spy.count > 1);
      verifyItems("items", [
          { url: "http://testsuite/tst_NavigationHistory1.html", originalUrl: "http://testsuite/tst_NavigationHistory1.html", title: "Navigation test 1" },
          { url: "http://testsuite/tst_NavigationHistory4.html", originalUrl: "http://testsuite/tst_NavigationHistory4.html", title: "Navigation test 4" }
      ], "Step 7");
    }

    function test_NavigationHistory5_currentItem() {
      compare(webView.navigationHistory.currentItem, null);

      loadUrl("http://testsuite/tst_NavigationHistory1.html");

      verify(spy.count > 0);
      verify(webView.navigationHistory.currentItem != null);
      compare(webView.navigationHistory.currentItem.url, "http://testsuite/tst_NavigationHistory1.html");
      compare(webView.navigationHistory.currentItem.originalUrl, "http://testsuite/tst_NavigationHistory1.html");
      compare(webView.navigationHistory.currentItem.title, "Navigation test 1");

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory2.html");

      verify(spy.count > 0);
      verify(webView.navigationHistory.currentItem != null);
      compare(webView.navigationHistory.currentItem.url, "http://testsuite/tst_NavigationHistory2.html");
      compare(webView.navigationHistory.currentItem.originalUrl, "http://testsuite/tst_NavigationHistory2.html");
      compare(webView.navigationHistory.currentItem.title, "Navigation test 2");

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory3.html");

      verify(spy.count > 0);
      verify(webView.navigationHistory.currentItem != null);
      compare(webView.navigationHistory.currentItem.url, "http://testsuite/tst_NavigationHistory3.html");
      compare(webView.navigationHistory.currentItem.originalUrl, "http://testsuite/tst_NavigationHistory3.html");
      compare(webView.navigationHistory.currentItem.title, "");

      spy.clear();
      webView.navigationHistory.currentItem = webView.navigationHistory.backItems[1];
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 0);
      verify(webView.navigationHistory.currentItem != null);
      compare(webView.navigationHistory.currentItem.url, "http://testsuite/tst_NavigationHistory1.html");
      compare(webView.navigationHistory.currentItem.originalUrl, "http://testsuite/tst_NavigationHistory1.html");
      compare(webView.navigationHistory.currentItem.title, "Navigation test 1");
      compare(webView.url, "http://testsuite/tst_NavigationHistory1.html");
      compare(webView.getTestApi().documentURI, "http://testsuite/tst_NavigationHistory1.html");

      var item = webView.navigationHistory.forwardItems[1];

      loadUrl("http://testsuite/tst_NavigationHistory4.html");

      // |item| is now no longer part of this navigation history. Ensure we handle this case
      spy.clear();
      webView.clearLoadEventCounters();
      webView.navigationHistory.currentItem = item;
      TestSupport.wait(500);

      compare(webView.loadsStartedCount, 0);
      compare(spy.count, 0);

      var otherWebView = webViewComponent.createObject(top, { "anchors.fill": parent });
      otherWebView.url = "http://testsuite/tst_NavigationHistory1.html";
      verify(otherWebView.waitForLoadSucceeded());

      webView.navigationHistory.currentItem = otherWebView.navigationHistory.currentItem;
      TestSupport.wait(500);

      compare(webView.loadsStartedCount, 0);
      compare(spy.count, 0);

      TestSupport.destroyQObjectNow(otherWebView);
    }

    function test_NavigationHistory6_currentItemIndex() {
      compare(webView.navigationHistory.currentItemIndex, -1);

      loadUrl("http://testsuite/tst_NavigationHistory1.html");

      verify(spy.count > 0);
      compare(webView.navigationHistory.currentItemIndex, 0);

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory2.html");

      verify(spy.count > 0);
      compare(webView.navigationHistory.currentItemIndex, 1);

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory3.html");

      verify(spy.count > 0);
      compare(webView.navigationHistory.currentItemIndex, 2);

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory4.html");

      verify(spy.count > 0);
      compare(webView.navigationHistory.currentItemIndex, 3);

      spy.clear();
      webView.navigationHistory.goBack();
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 0);
      compare(webView.navigationHistory.currentItemIndex, 2);

      spy.clear();
      webView.clearLoadEventCounters();
      webView.navigationHistory.currentItemIndex = 10;
      TestSupport.wait(500);

      compare(webView.loadsStartedCount, 0);
      compare(spy.count, 0);

      webView.navigationHistory.currentItemIndex = -1;
      TestSupport.wait(500);

      compare(webView.loadsStartedCount, 0);
      compare(spy.count, 0);

      webView.navigationHistory.currentItemIndex = 1;
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 0);
      compare(webView.navigationHistory.currentItemIndex, 1);
    }

    function test_NavigationHistory7_goBack() {
      verify(!webView.navigationHistory.canGoBack);

      // Shouldn't crash
      webView.navigationHistory.goBack();

      loadUrl("http://testsuite/tst_NavigationHistory1.html");

      verify(!webView.navigationHistory.canGoBack);
      webView.navigationHistory.goBack();

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory2.html");

      verify(spy.count > 0);
      verify(webView.navigationHistory.canGoBack);

      loadUrl("http://testsuite/tst_NavigationHistory3.html");
      loadUrl("http://testsuite/tst_NavigationHistory4.html");

      verify(webView.navigationHistory.canGoBack);

      var expectedItem = webView.navigationHistory.backItems[0];
      spy.clear();
      webView.navigationHistory.goBack();
      verify(webView.waitForLoadSucceeded());

      compare(webView.url, "http://testsuite/tst_NavigationHistory3.html");
      compare(webView.getTestApi().documentURI, "http://testsuite/tst_NavigationHistory3.html");
      compare(webView.navigationHistory.currentItem, expectedItem);
      verify(spy.count > 0);
      verify(webView.navigationHistory.canGoBack);

      expectedItem = webView.navigationHistory.backItems[0];
      spy.clear();
      webView.navigationHistory.goBack();
      verify(webView.waitForLoadSucceeded());

      compare(webView.url, "http://testsuite/tst_NavigationHistory2.html");
      compare(webView.getTestApi().documentURI, "http://testsuite/tst_NavigationHistory2.html");
      compare(webView.navigationHistory.currentItem, expectedItem);
      verify(spy.count > 0);
      verify(webView.navigationHistory.canGoBack);

      expectedItem = webView.navigationHistory.backItems[0];
      spy.clear();
      webView.navigationHistory.goBack();
      verify(webView.waitForLoadSucceeded());

      compare(webView.url, "http://testsuite/tst_NavigationHistory1.html");
      compare(webView.getTestApi().documentURI, "http://testsuite/tst_NavigationHistory1.html");
      compare(webView.navigationHistory.currentItem, expectedItem);
      verify(spy.count > 0);
      verify(!webView.navigationHistory.canGoBack);

      // Shouldn't crash
      webView.navigationHistory.goBack();
    }

    function test_NavigationHistory8_goForward() {
      verify(!webView.navigationHistory.canGoForward);

      // Shouldn't crash
      webView.navigationHistory.goForward();

      loadUrl("http://testsuite/tst_NavigationHistory1.html");
      loadUrl("http://testsuite/tst_NavigationHistory2.html");
      loadUrl("http://testsuite/tst_NavigationHistory3.html");
      loadUrl("http://testsuite/tst_NavigationHistory4.html");

      verify(!webView.navigationHistory.canGoForward);
      webView.navigationHistory.goForward();

      spy.clear();
      webView.navigationHistory.goBack();
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 0);
      verify(webView.navigationHistory.canGoForward);

      while (webView.navigationHistory.canGoBack) {
        webView.navigationHistory.goBack();
        verify(webView.waitForLoadSucceeded());
      }

      verify(webView.navigationHistory.canGoForward);

      var expectedItem = webView.navigationHistory.forwardItems[0];
      spy.clear();
      webView.navigationHistory.goForward();
      verify(webView.waitForLoadSucceeded());

      compare(webView.url, "http://testsuite/tst_NavigationHistory2.html");
      compare(webView.getTestApi().documentURI, "http://testsuite/tst_NavigationHistory2.html");
      compare(webView.navigationHistory.currentItem, expectedItem);
      verify(spy.count > 0);
      verify(webView.navigationHistory.canGoForward);

      expectedItem = webView.navigationHistory.forwardItems[0];
      spy.clear();
      webView.navigationHistory.goForward();
      verify(webView.waitForLoadSucceeded());

      compare(webView.url, "http://testsuite/tst_NavigationHistory3.html");
      compare(webView.getTestApi().documentURI, "http://testsuite/tst_NavigationHistory3.html");
      compare(webView.navigationHistory.currentItem, expectedItem);
      verify(spy.count > 0);
      verify(webView.navigationHistory.canGoForward);

      expectedItem = webView.navigationHistory.forwardItems[0];
      spy.clear();
      webView.navigationHistory.goForward();
      verify(webView.waitForLoadSucceeded());

      compare(webView.url, "http://testsuite/tst_NavigationHistory4.html");
      compare(webView.getTestApi().documentURI, "http://testsuite/tst_NavigationHistory4.html");
      compare(webView.navigationHistory.currentItem, expectedItem);
      verify(spy.count > 0);
      verify(!webView.navigationHistory.canGoForward);

      // Shouldn't crash
      webView.navigationHistory.goForward();

      while (webView.navigationHistory.canGoBack) {
        webView.navigationHistory.goBack();
        verify(webView.waitForLoadSucceeded());
      }

      verify(webView.navigationHistory.canGoForward);

      spy.clear();
      loadUrl("http://testsuite/tst_NavigationHistory4.html");

      verify(spy.count > 0);
      verify(!webView.navigationHistory.canGoForward);
    }

    function test_NavigationHistory9_goToOffset() {
      loadUrl("http://testsuite/tst_NavigationHistory1.html");
      loadUrl("http://testsuite/tst_NavigationHistory2.html");
      loadUrl("http://testsuite/tst_NavigationHistory3.html");
      loadUrl("http://testsuite/tst_NavigationHistory4.html");

      spy.clear();
      var expectedItem = webView.navigationHistory.backItems[1];

      webView.navigationHistory.goToOffset(-2);
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 0);
      compare(webView.url, "http://testsuite/tst_NavigationHistory2.html");
      compare(webView.getTestApi().documentURI, "http://testsuite/tst_NavigationHistory2.html");
      compare(webView.navigationHistory.currentItem, expectedItem);

      spy.clear();
      expectedItem = webView.navigationHistory.forwardItems[0];

      webView.navigationHistory.goToOffset(1);
      verify(webView.waitForLoadSucceeded());

      verify(spy.count > 0);
      compare(webView.url, "http://testsuite/tst_NavigationHistory3.html");
      compare(webView.getTestApi().documentURI, "http://testsuite/tst_NavigationHistory3.html");
      compare(webView.navigationHistory.currentItem, expectedItem);

      webView.clearLoadEventCounters();
      spy.clear();

      webView.navigationHistory.goToOffset(10);
      TestSupport.wait(500);

      compare(spy.count, 0);
      compare(webView.loadsStartedCount, 0);

      webView.navigationHistory.goToOffset(0);
      TestSupport.wait(500);

      compare(spy.count, 0);
      compare(webView.loadsStartedCount, 0);
    }
  }
}
