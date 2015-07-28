import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  Component {
    id: webViewFactory
    TestWebView {}
  }

  property var created: null

  onNewViewRequested: {       
    created = webViewFactory.createObject(webView, { request: request });
  }

  TestCase {
    id: test
    name: "bug1477760"
    when: windowShown

    function test_bug1477760() {
      webView.url = "http://testsuite/tst_nav.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      mouseClick(webView, webView.width / 2, webView.height / 2);

      TestUtils.waitFor(function() { return !!webView.created; });
    }
  }
}
