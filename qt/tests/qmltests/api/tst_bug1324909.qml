import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

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
    name: "bug1324909"
    when: windowShown

    function test_bug1324909_1_new_window() {
      verify(webView.context.userAgent != "Foo");
      webView.context.userAgent = "Foo";

      webView.url = "http://testsuite/tst_bug1324909_1.html";
      verify(webView.waitForLoadSucceeded());

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      spy.wait();
      verify(created != null);

      compare(created.getTestApi().evaluateCode(
          "return document.getElementById(\"useragent\").innerHTML;", true),
          "Foo");
    }

    function test_bug1324909_2_subframe() {
      webView.url = "http://testsuite/tst_bug1324909_2.html";
      verify(webView.waitForLoadSucceeded());

      console.log(webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).documentURI);
      compare(webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode(
          "return document.getElementById(\"useragent\").innerHTML;", true),
          "Foo");
    }
  }
}
