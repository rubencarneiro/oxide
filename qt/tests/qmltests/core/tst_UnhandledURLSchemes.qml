import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  SignalSpy {
    id: urlHandledSpy
    target: OxideTestingUtils
    signalName: "urlHandled"
  }

  TestCase {
    name: "UnhandledURLSchemes"
    when: windowShown

    function init() {
      OxideTestingUtils.setUrlHandler("customscheme", true);
      urlHandledSpy.clear();
    }

    function cleanup() {
      OxideTestingUtils.unsetUrlHandler("customscheme");
    }

    function test_UnhandledURLSchemes_handled_by_system() {
      webView.url = "http://testsuite/tst_UnhandledURLSchemes1.html";
      verify(webView.waitForLoadSucceeded());

      for (var i = 1; i <= 5; ++i) {
        mouseClick(webView, webView.width / 2, webView.height / 2);
        webView.waitForLoadStopped();
        urlHandledSpy.wait();
        compare(urlHandledSpy.signalArguments[i - 1][0].toString(),
                "customscheme:test" + i);
      }
    }

    function test_UnhandledURLSchemes_not_handled_by_system() {
      webView.url = "http://testsuite/tst_UnhandledURLSchemes2.html";
      verify(webView.waitForLoadSucceeded());

      OxideTestingUtils.setUrlHandler("customscheme", true);
      mouseClick(webView, webView.width / 2, webView.height / 2);
      webView.waitForLoadStopped();
      compare(urlHandledSpy.count, 0);
    }
  }
}
