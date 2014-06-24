import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: contentXSpy
    target: webView
    signalName: "contentXChanged"
  }

  SignalSpy {
    id: contentYSpy
    target: webView
    signalName: "contentYChanged"
  }

  TestCase {
    name: "WebView_flickableLikeAPI"
    when: windowShown

    function get(attr) {
      return parseFloat(webView.getTestApi().evaluateCode(attr));  
    }

    function set(attr, value) {
      webView.getTestApi().evaluateCode(attr + " = " + value.toString());
    }

    function test_WebView_flickableLikeAPI() {
      webView.url = "http://localhost:8080/tst_WebView_flickableLikeAPI.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
      compare(get("document.body.clientWidth"), webView.viewportWidth);
      compare(get("document.body.clientHeight"), webView.viewportHeight);
      compare(get("document.body.scrollWidth"), webView.contentWidth);
      compare(get("document.body.scrollHeight"), webView.contentHeight);
      compare(get("document.body.scrollLeft"), webView.contentX);
      compare(get("document.body.scrollTop"), webView.contentY);

      set("document.body.scrollLeft", 200);
      contentXSpy.wait();
      compare(get("document.body.scrollLeft"), webView.contentX);

      set("document.body.scrollTop", 500);
      contentYSpy.wait();
      compare(get("document.body.scrollTop"), webView.contentY);

      set("document.body.scrollLeft", 2500);
      contentXSpy.wait();
      compare(get("document.body.scrollLeft"), webView.contentX);
      compare(webView.contentX, webView.contentWidth - webView.viewportWidth);

      set("document.body.scrollTop", 2500);
      contentYSpy.wait();
      compare(get("document.body.scrollTop"), webView.contentY);
      compare(webView.contentY, webView.contentHeight - webView.viewportHeight);
    }
  }
}
