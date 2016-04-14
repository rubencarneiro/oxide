import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.3
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  // XXX: Work around https://launchpad.net/bugs/1389721
  Component.onCompleted: {
    context.allowedExtraUrlSchemes = [ "test", "bar" ];
  }

  TestCase {
    name: "CustomURLSchemes"
    when: windowShown

    // This test loads the same text file twice - once using Chromium's file
    // protocol handler and once using a custom handler - and compares their
    // content. This is a fairly primitive test at the moment - eg, we don't test
    // the content type (underneath, our custom handler uses Qt's file protocol
    // handler - which doesn't set the content type). The text file is sufficiently
    // large to test the cross thread buffering

    function test_CustomURLSchemes1_compare_content() {
      webView.url = "test:///tst_CustomURLSchemes.txt";
      verify(webView.waitForLoadSucceeded(20000));

      var test = webView.getTestApi().evaluateCode("return document.body.children[0].innerHTML;", true);

      webView.url = Qt.resolvedUrl("tst_CustomURLSchemes.txt");
      verify(webView.waitForLoadSucceeded(20000));

      var file = webView.getTestApi().evaluateCode("return document.body.children[0].innerHTML;", true);

      compare(test, file);
    }

    function test_CustomURLSchemes2_invalid_scheme() {
      webView.url = "bar:///tst_CustomURLSchemes.txt";
      verify(webView.waitForLoadFailed());
    }

    function test_CustomURLSchemes3_disallowed_scheme() {
      webView.context.allowedExtraUrlSchemes = [];
      webView.url = "test:///tst_CustomURLSchemes.txt";
      verify(webView.waitForLoadStopped());
    }
  }
}
