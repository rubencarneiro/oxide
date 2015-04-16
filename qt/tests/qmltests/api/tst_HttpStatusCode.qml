import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  TestCase {
    id: test
    name: "HttpStatusCode"
    when: windowShown

    function init() {
      webView.clearLoadEventCounters();
    }

    function test_HttpStatusCode404() {
      function _onLoadEvent(event) {
        compare(event.httpStatusCode, LoadEvent.HttpStatusCodeNotFound);
      }

      webView.loadEvent.connect(_onLoadEvent);

      webView.url = "http://testsuite/404Page";
      verify(webView.waitForLoadCommitted());

      webView.loadEvent.disconnect(_onLoadEvent);
    }
  }
}
