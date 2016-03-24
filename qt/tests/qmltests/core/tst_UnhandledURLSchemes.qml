import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  ExternalProtocolHandler {
    id: protocolHandler
    scheme: "customscheme"
  }

  SignalSpy {
    id: urlHandledSpy
    target: protocolHandler
    signalName: "openUrl"
  }

  TestCase {
    name: "UnhandledURLSchemes"
    when: windowShown

    function init() {
      urlHandledSpy.clear();
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

      mouseClick(webView, webView.width / 2, webView.height / 2);
      webView.waitForLoadStopped();
      compare(urlHandledSpy.count, 0);
    }
  }
}
