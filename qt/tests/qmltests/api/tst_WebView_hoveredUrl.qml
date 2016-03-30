import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.12
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "hoveredUrlChanged"
  }

  TestCase {
    name: "WebView_hoveredUrl"
    when: windowShown

    function test_hoveredUrl() {
      webView.url = "http://foo.testsuite/tst_WebView_hoveredUrl.html"
      verify(webView.waitForLoadSucceeded());
      spy.clear();

      var r = webView.getTestApi().getBoundingClientRectForSelector("#link1");
      mouseMove(webView, r.x + r.width / 2, r.y + r.height / 2);
      spy.wait();
      compare(webView.hoveredUrl, "http://example.org/");
      compare(spy.count, 1);

      r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseMove(webView, r.x + r.width / 2, r.y + r.height / 2);
      spy.wait();
      compare(webView.hoveredUrl, "");
      compare(spy.count, 2);

      r = webView.getTestApi().getBoundingClientRectForSelector("#link2");
      mouseMove(webView, r.x + r.width / 2, r.y + r.height / 2);
      spy.wait();
      compare(webView.hoveredUrl, "https://launchpad.net/");
      compare(spy.count, 3);

      mouseMove(webView, r.x + r.width / 2, r.y + r.height * 2);
      spy.wait();
      compare(webView.hoveredUrl, "");
      compare(spy.count, 4);
    }
  }
}
