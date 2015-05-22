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
    name: "findController"
    when: windowShown

    function init() {
      webView.findController.text = "";
      webView.findController.caseSensitive = false;

      // load the page that will be used by most tests
      webView.url = "http://testsuite/tst_WebView_findController.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }

    function test_case_insensitive() {
      webView.findController.text = "suspendisse";
      tryCompare(webView.findController, "count", 1);

      webView.findController.text = "aMet";
      tryCompare(webView.findController, "count", 3);
    }

    function test_case_sensitive() {
      webView.findController.caseSensitive = true;

      webView.findController.text = "Suspendisse";
      tryCompare(webView.findController, "count", 1);

      webView.findController.text = "suspendisse";
      tryCompare(webView.findController, "count", 0);
    }

    function test_change_case_sensitivity() {
      webView.findController.caseSensitive = true;

      webView.findController.text = "Dolor";
      tryCompare(webView.findController, "count", 1);

      webView.findController.caseSensitive = false;
      tryCompare(webView.findController, "count", 2);
    }

    function test_movement() {
      webView.findController.text = "dolor";
      tryCompare(webView.findController, "count", 2);
      tryCompare(webView.findController, "current", 1);

      webView.findController.next();
      tryCompare(webView.findController, "count", 2);
      tryCompare(webView.findController, "current", 2);

      webView.findController.previous();
      tryCompare(webView.findController, "count", 2);
      tryCompare(webView.findController, "current", 1);
    }

    function test_movement_wraps() {
      webView.findController.text = "dolor";
      tryCompare(webView.findController, "count", 2);
      tryCompare(webView.findController, "current", 1);

      webView.findController.next();
      webView.findController.next();
      tryCompare(webView.findController, "current", 1);

      webView.findController.previous();
      webView.findController.previous();
      tryCompare(webView.findController, "current", 1);
    }

    function test_clear() {
      webView.findController.text = "dolor";
      tryCompare(webView.findController, "count", 2);

      webView.findController.text = "";
      tryCompare(webView.findController, "count", 0);
      tryCompare(webView.findController, "current", 0);
    }

    function test_new_text_resets_count() {
      webView.findController.text = "dolor";
      tryCompare(webView.findController, "count", 2);
      webView.findController.next();
      tryCompare(webView.findController, "current", 2);

      webView.findController.text = "suspendisse";
      tryCompare(webView.findController, "count", 1);
      tryCompare(webView.findController, "current", 1);
    }

    function test_not_found_resets_count() {
      webView.findController.text = "dolor";
      tryCompare(webView.findController, "count", 2);

      webView.findController.text = "i_am_not_there";
      tryCompare(webView.findController, "count", 0);
      tryCompare(webView.findController, "current", 0);
    }

    function test_find_non_latin() {
      webView.findController.text = "ñec";
      tryCompare(webView.findController, "count", 1);
    }

    function test_find_on_invalid_page() {
      webView.url = "http://testsuite/tst_invalid_page.html";
      // verify successful load because the server returns a 404 page apparently
      // and that does not count as a failure
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.findController.text = "hello";
      tryCompare(webView.findController, "count", 0);
      tryCompare(webView.findController, "current", 0);
    }

    function test_navigation_does_not_reset() {
      webView.findController.text = "dolor";
      tryCompare(webView.findController, "count", 2);
      webView.findController.next();
      tryCompare(webView.findController, "current", 2);

      // Verify that when navigating to another page the search does not
      // reset, it is the user responsibility to cancel it and restart it
      webView.url = "http://testsuite/tst_WebView_findControllerManyResults.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compare(webView.findController.text, "dolor");
      tryCompare(webView.findController, "count", 2);
      tryCompare(webView.findController, "current", 2);

      webView.findController.text = "";
      tryCompare(webView.findController, "count", 0);
      tryCompare(webView.findController, "current", 0);

      webView.findController.text = "dolor";
      tryCompare(webView.findController, "count", 20);
      tryCompare(webView.findController, "current", 1);
    }
  }
}
