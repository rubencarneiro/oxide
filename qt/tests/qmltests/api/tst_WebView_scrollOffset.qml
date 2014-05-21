import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "scrollOffsetChanged"
  }

  TestCase {
    name: "WebView_scrollOffset"
    when: windowShown

    function get(attr) {
      return parseFloat(webView.getTestApi().evaluateCode(attr));  
    }

    function set(attr, value) {
      webView.getTestApi().evaluateCode(attr + " = " + value.toString());
    }

    function test_WebView_scrollOffset() {
      webView.url = "http://localhost:8080/tst_WebView_scrollOffset.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
      compare(spy.count, 0);
      compare(get("document.body.clientWidth"), webView.viewportSize.width);
      compare(get("document.body.clientHeight"), webView.viewportSize.height);
      compare(get("document.body.scrollWidth"), webView.layerSize.width);
      compare(get("document.body.scrollHeight"), webView.layerSize.height);
      compare(get("document.body.scrollLeft"), webView.scrollOffset.x);
      compare(get("document.body.scrollTop"), webView.scrollOffset.y);

      set("document.body.scrollLeft", 200);
      spy.wait();
      compare(get("document.body.scrollLeft"), webView.scrollOffset.x);

      set("document.body.scrollTop", 500);
      spy.wait();
      compare(get("document.body.scrollTop"), webView.scrollOffset.y);

      set("document.body.scrollLeft", 2500);
      spy.wait();
      compare(get("document.body.scrollLeft"), webView.scrollOffset.x);
      compare(webView.scrollOffset.x,
              webView.layerSize.width - webView.viewportSize.width);

      set("document.body.scrollTop", 2500);
      spy.wait();
      compare(get("document.body.scrollTop"), webView.scrollOffset.y);
      compare(webView.scrollOffset.y,
              webView.layerSize.height - webView.viewportSize.height);
    }
  }
}
