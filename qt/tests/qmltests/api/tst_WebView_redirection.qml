import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "loadingChanged"
  }

  TestCase {
    name: "WebView_redirection"
    when: windowShown

    function test_WebView_redirection() {
      function onLoadingChanged(loadEvent) {
        var url;
        if (loadEvent.type === LoadEvent.TypeStarted) {
          url = "http://testsuite/tst_WebView_redirection.py";
        } else if (loadEvent.type === LoadEvent.TypeSucceeded) {
          url = "http://testsuite/empty.html";
        } else {
          fail("Unexpected load event");
        }
        compare(webView.url, url);
        compare(loadEvent.url, url);
      }
      webView.loadingChanged.connect(onLoadingChanged);
      compare(spy.count, 0);
      webView.url = "http://testsuite/tst_WebView_redirection.py";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
      compare(spy.count, 2);
    }
  }
}
