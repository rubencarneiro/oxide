import QtQuick 2.0
import QtTest 1.0
import "../../utils"

TestWebView {
  id: webView
  focus: true

  property var titleChanged: false

  TestCase {
    id: test
    name: "WebView_title"
    when: windowShown

    function test_WebView_title() {
      compare(webView.title, "");

      webView.url = "http://localhost:8080/title.html";
      verify(webView.waitForLoadSucceeded());

      compare(webView.title, "Test");

      var test = "This is a test title";

      titleChanged = false;
      webView.getTestApi().documentTitle = test;
      verify(waitFor(function() { return titleChanged == true; }));

      compare(webView.title, test);
    }
  }

  onTitleChanged: {
    titleChanged = true;
  }
}
