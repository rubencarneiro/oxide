import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "titleChanged"
  }

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
      compare(spy.count, 2,
              "There should have been 2 title changes during the load");

      var test = "This is a test title";

      spy.clear();
      webView.getTestApi().evaluateCode("window.document.title = \"" + test + "\"");
      spy.wait();

      compare(webView.title, test,
              "WebView.title should match the new title");

      spy.clear();
      webView.reload();
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful reload");

      compare(webView.title, "Test",
              "WebView.title should match the document title after a reload");
      compare(spy.count, 2,
              "There should have been 2 title changes during the reload");
    }
  }
}
