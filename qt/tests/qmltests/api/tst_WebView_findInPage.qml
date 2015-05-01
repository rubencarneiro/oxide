import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.8
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  TestCase {
    name: "findInPage"
    when: windowShown

    function init() {
      webView.findInPage.text = "";
      webView.findInPage.caseSensitive = false;

      // load the page that will be used by most tests
      webView.url = "http://testsuite/tst_WebView_findInPage.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }

    function test_case_insensitive() {
      webView.findInPage.text = "suspendisse";
      tryCompare(webView.findInPage, "count", 1);

      webView.findInPage.text = "suspendiSSe";
      tryCompare(webView.findInPage, "count", 1);
    }

    function test_case_sensitive() {
      webView.findInPage.caseSensitive = true;

      webView.findInPage.text = "Suspendisse";
      tryCompare(webView.findInPage, "count", 1);

      webView.findInPage.text = "suspendisse";
      tryCompare(webView.findInPage, "count", 0);
    }

    function test_change_case_sensitivity() {
      webView.findInPage.caseSensitive = true;

      webView.findInPage.text = "Dolor";
      tryCompare(webView.findInPage, "count", 1);

      webView.findInPage.caseSensitive = false;
      tryCompare(webView.findInPage, "count", 2);
    }

    function test_movement() {
      webView.findInPage.text = "dolor";
      tryCompare(webView.findInPage, "count", 2);
      tryCompare(webView.findInPage, "current", 1);

      webView.findInPage.next();
      tryCompare(webView.findInPage, "count", 2);
      tryCompare(webView.findInPage, "current", 2);

      webView.findInPage.previous();
      tryCompare(webView.findInPage, "count", 2);
      tryCompare(webView.findInPage, "current", 1);
    }

    function test_movement_wraps() {
      webView.findInPage.text = "dolor";
      tryCompare(webView.findInPage, "count", 2);
      tryCompare(webView.findInPage, "current", 1);

      webView.findInPage.next();
      webView.findInPage.next();
      tryCompare(webView.findInPage, "current", 1);

      webView.findInPage.previous();
      webView.findInPage.previous();
      tryCompare(webView.findInPage, "current", 1);
    }

    function test_clear() {
      webView.findInPage.text = "dolor";
      tryCompare(webView.findInPage, "count", 2);

      webView.findInPage.text = "";
      tryCompare(webView.findInPage, "count", 0);
      tryCompare(webView.findInPage, "current", 0);
    }

    function test_new_text_resets_count() {
      webView.findInPage.text = "dolor";
      webView.findInPage.next();
      tryCompare(webView.findInPage, "count", 2);
      tryCompare(webView.findInPage, "current", 2);

      webView.findInPage.text = "suspendisse";
      tryCompare(webView.findInPage, "count", 1);
      tryCompare(webView.findInPage, "current", 1);
    }

    function test_not_found_resets_count() {
      webView.findInPage.text = "dolor";
      tryCompare(webView.findInPage, "count", 2);

      webView.findInPage.text = "i_am_not_there";
      tryCompare(webView.findInPage, "count", 0);
      tryCompare(webView.findInPage, "current", 0);
    }

    function test_find_non_latin() {
      webView.findInPage.text = "ñec";
      tryCompare(webView.findInPage, "count", 1);
    }

    function test_find_on_invalid_page() {
      webView.url = "http://testsuite/tst_invalid_page.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.findInPage.text = "hello";
      tryCompare(webView.findInPage, "count", 0);
      tryCompare(webView.findInPage, "current", 0);
    }

    function test_navigation_does_not_reset() {
      webView.findInPage.text = "dolor";
      webView.findInPage.next();
      tryCompare(webView.findInPage, "count", 2);
      tryCompare(webView.findInPage, "current", 2);

      // Verify that when navigating to another page the search does not
      // reset, it is the user responsibility to cancel it and restart it
      webView.url = "http://testsuite/tst_WebView_findInPageManyResults.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      tryCompare(webView.findInPage, "text", "dolor");
      tryCompare(webView.findInPage, "count", 2);
      tryCompare(webView.findInPage, "current", 2);

      webView.findInPage.text = "";
      tryCompare(webView.findInPage, "count", 0);
      tryCompare(webView.findInPage, "current", 0);

      webView.findInPage.text = "dolor";
      tryCompare(webView.findInPage, "count", 20);
      tryCompare(webView.findInPage, "current", 1);
    }
  }
}
