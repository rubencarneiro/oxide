import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0
import Qt.test.qtestroot 1.0 as TestRoot

TestWebView {
  id: webView
  width: 200
  height: 200

  SignalSpy {
    id: loadSpy
    target: webView
    signalName: "loadEvent"
  }

  SignalSpy {
    id: urlHandledSpy
    target: TestRoot.QTestRootObject
    signalName: "urlHandled"
  }

  TestCase {
    name: "UnhandledURLSchemes"
    when: windowShown

    function test_UnhandledURLSchemes_handled_by_system() {
      webView.url = "http://testsuite/tst_UnhandledURLSchemes1.html";
      verify(webView.waitForLoadSucceeded());
      urlHandledSpy.clear();

      for (var i = 1; i <= 5; ++i) {
        mouseClick(webView, webView.width / 2, webView.height / 2);
        urlHandledSpy.wait();
        compare(urlHandledSpy.signalArguments[i - 1][0].toString(),
                "customscheme:test" + i);
      }
    }

    function test_UnhandledURLSchemes_not_handled_by_system() {
      webView.url = "http://testsuite/tst_UnhandledURLSchemes2.html";
      verify(webView.waitForLoadSucceeded());
      urlHandledSpy.clear();

      mouseClick(webView, webView.width / 2, webView.height / 2);
      loadSpy.wait(); // load started
      loadSpy.wait(); // load stopped
      compare(urlHandledSpy.count, 0);
    }
  }
}
