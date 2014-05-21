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

    function test_WebView_loadHtml_data() {
      return [
        { baseUrl: Qt.resolvedUrl("."), url: Qt.resolvedUrl(".") },
        { baseUrl: "", url: "about:blank" },
        { baseUrl: "file:///", url: "file:///" },
        { baseUrl: "file://", url: "file:///" },
        { baseUrl: "file:/", url: "file:///" },
        { baseUrl: "file:", url: "file:///" },
        { baseUrl: "file", url: "about:blank" },
        { baseUrl: "http://localhost:8080/", url: "http://localhost:8080/" }
      ];
    }

    function test_WebView_loadHtml(data) {
      var html = '<html><body><div id="content">OK</div></body></html>';
      webView.loadHtml(html, data.baseUrl);
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
      compare(webView.url, data.url);
      var content = webView.getTestApi().evaluateCode(
          'document.querySelector("#content").innerHTML');
      compare(content, "OK");
    }
  }
}
