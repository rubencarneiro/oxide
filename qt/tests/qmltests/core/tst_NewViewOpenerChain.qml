import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

Column {
  id: column
  focus: true

  Component {
    id: webViewFactory
    TestWebView {}
  }

  TestWebView {
    id: webView
    width: 200
    height: 200

    property var created: null

    onNewViewRequested: {
      created = webViewFactory.createObject(column, { request: request, width: 200, height: 200 });
    }
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "newViewRequested"
  }

  TestCase {
    id: test
    name: "NewViewOpenerChain"
    when: windowShown

    function test_NewViewOpenerChain1() {
      webView.url = "http://localhost:8080/tst_NewViewOpenerChain.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      spy.wait();

      verify(webView.created.getTestApi().evaluateCode("return window.opener != null;", true));
    }
  }
}
