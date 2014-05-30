import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

Column {
  id: column
  focus: true

  Component {
    id: webViewFactory
    TestWebView {}
  }

  property var created: null

  TestWebView {
    id: webView
    focus: true
    width: 200
    height: 200

    onNewViewRequested: {
      created = webViewFactory.createObject(column, { request: request });
    }
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "newViewRequested"
  }

  TestCase {
    id: test
    name: "bugXXXXXXX"
    when: windowShown

    function test_bugXXXXXXX() {
      verify(webView.context.userAgent != "Foo");
      webView.context.userAgent = "Foo";

      webView.url = "http://localhost:8080/tst_bug1324909_1.html";
      verify(webView.waitForLoadSucceeded());

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      spy.wait();
      verify(created != null);

      skip("This test is known to fail");
      //compare(created.getTestApi().evaluateCode(
      //    "return document.getElementById(\"useragent\").innerHTML;", true),
      //    "Foo");
    }
  }
}
