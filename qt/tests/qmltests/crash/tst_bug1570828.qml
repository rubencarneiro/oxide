import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

Item {
  id: top

  Component {
    id: webViewFactory
    TestWebView {}
  }

  TestCase {
    id: test
    name: "bug1570828"
    when: windowShown

    function initTestCase() {
      SingletonTestWebContext.addTestUserScript({
          context: "oxide://bug1570828test/",
          url: Qt.resolvedUrl("tst_bug1570828.js")
      });
    }

    function cleanupTestCase() {
      SingletonTestWebContext.clearTestUserScripts();
    }

    function test_bug1570828() {
      for (var i = 0; i < 50; ++i) {
        var webView = webViewFactory.createObject(top, {});
        webView.url = "http://testsuite/tst_bug1570828.html";
        verify(webView.waitForLoadSucceeded());

        TestSupport.destroyQObjectNow(webView);
      }
    }
  }
}
