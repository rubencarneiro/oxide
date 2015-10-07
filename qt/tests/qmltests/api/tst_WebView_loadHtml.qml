import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  Component {
    id: webViewComponent
    TestWebView {
      context: TestWebContext { persistent: false }
      property string html: ""
      onHtmlChanged: loadHtml(html, "file:///")
    }
  }

  onHtmlChanged: console.log("BAA")
  property string html: "<html></html>"

  TestCase {
    name: "WebView_loadHtml"
    when: windowShown

    function test_WebView_loadHtml1_data() {
      return [
        { baseUrl: Qt.resolvedUrl("."), url: Qt.resolvedUrl(".") },
        { baseUrl: "", url: "about:blank" },
        { baseUrl: "file:///", url: "file:///" },
        { baseUrl: "file://", url: "file:///" },
        { baseUrl: "file:/", url: "file:///" },
        { baseUrl: "file:", url: "file:///" },
        { baseUrl: "file", url: "about:blank" },
        { baseUrl: "http://testsuite/", url: "http://testsuite/" }
      ];
    }

    function test_WebView_loadHtml1(data) {
      var html = '<html><body><div id="content">OK</div></body></html>';
      webView.loadHtml(html, data.baseUrl);
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
      compare(webView.url, data.url);
      var content = webView.getTestApi().evaluateCode(
          'document.querySelector("#content").innerHTML');
      compare(content, "OK");
    }

    function test_WebView_loadHtml2() {
      var html = '<html><body><div id="content">OK</div></body></html>';
      var view = webViewComponent.createObject(null, { html: html });
      verify(view.waitForLoadSucceeded());
      compare(view.url, "file:///");
      var content = webView.getTestApi().evaluateCode(
          'document.querySelector("#content").innerHTML');
      compare(content, "OK");
    }
  }
}
