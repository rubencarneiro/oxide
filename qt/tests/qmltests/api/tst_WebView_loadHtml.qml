import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  TestCase {
    name: "WebView_loadHtml"
    when: windowShown

    function test_WebView_loadHtml() {
      var html = '<html><body><div id="content">OK</div></body></html>';
      var baseUrl = Qt.resolvedUrl(".");
      webView.loadHtml(html, baseUrl);
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
      compare(webView.url, baseUrl);
      var content = webView.getTestApi().evaluateCode(
          'document.querySelector("#content").innerHTML');
      compare(content, "OK");
    }
  }
}
