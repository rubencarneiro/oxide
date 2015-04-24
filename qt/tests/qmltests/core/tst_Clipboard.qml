import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView

  focus: true

  width: 200
  height: 200

  function expect_content(expected) {
    var result = webView.getTestApi().evaluateCode(
        "document.querySelector('#content').value");
    return (result === expected);
  }

  function set_content(content) {
    webView.getTestApi().evaluateCode(
        "document.querySelector('#content').value = " + content);
  }

  TestCase {
    id: testcase
    name: "clipboard"

    when: windowShown

    function setup() {
      OxideTestingUtils.clearClipboard();
    }
    
    function test_paste_data() {
      return [
        { content: "content", mimeType: "text/plain" },
        { content: "<html><head></head><body></body></html>", mimeType: "text/html" },
      ];
    }

    function test_paste(data) {
      OxideTestingUtils.copyToClipboard(data.mimeType, data.content);

      webView.url = "http://testsuite/tst_Clipboard.html";

      verify(webView.waitForLoadSucceeded(), "Timed out waiting for successful load");

      keyPress("v", Qt.ControlModifier)

      verify(webView.waitFor(function() { return expect_content(data.content); }));
    }

    function test_copy_data() {
      return [
        { content: "content", mimeType: "text/plain" },
        { content: "<html><head></head><body></body></html>", mimeType: "text/html" },
      ];
    }

    function test_copy(data) {
      webView.url = "http://testsuite/tst_Clipboard.html";

      verify(webView.waitForLoadSucceeded(), "Timed out waiting for successful load");

      set_content(data.content)

      webView.waitFor(function() { return expect_content(data.content); });

      keyPress("c", Qt.ControlModifier)

      var current_content = OxideTestingUtils.copyFromClipboard(
          data.mimeType, data.content);

      verify(current_content === data.content)
    }
  }
}
