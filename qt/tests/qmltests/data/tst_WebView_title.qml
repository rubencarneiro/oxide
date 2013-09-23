import QtQuick 2.0
import QtTest 1.0
import "../../utils"

TestWebView {
  id: webView
  focus: true

  property var titleChangedCount: 0

  TestCase {
    id: test
    name: "WebView_title"
    when: windowShown

    function test_WebView_title() {
      compare(webView.title, "",
              "The title should be empty before we load anything");

      webView.url = "http://localhost:8080/tst_WebView_title.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      compare(webView.title, "Test",
              "WebView.title should match the document title");
      compare(titleChangedCount, 2,
              "There should have been 2 title changes during the load");

      var test = "This is a test title";

      titleChangedCount = 0;
      webView.getTestApi().documentTitle = test;
      verify(waitFor(function() { return titleChangedCount > 0; }),
             "Timed out waiting for the title to change");

      compare(webView.title, test,
              "WebView.title should match the new title");

      titleChangedCount = 0;
      webView.reload();
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful reload");

      compare(webView.title, "Test",
              "WebView.title should match the document title after a reload");
      compare(titleChangedCount, 2,
              "There should have been 2 title changes during the reload");
    }
  }

  onTitleChanged: {
    titleChangedCount++;
  }
}
