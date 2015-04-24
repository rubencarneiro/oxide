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
        "return document.querySelector('#content').value", true);
    return (result === expected);
  }

  function select_textarea_content() {
    webView.getTestApi().evaluateCode(
        "document.querySelector('#content').select()", true);
  }

  function set_content(content) {
    webView.getTestApi().evaluateCode(
        "document.querySelector('#content').value = '" + content + "'", true);
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
        { content: "content", mimeType: "text/plain", isimage: false },
        { content: OxideTestingUtils.getClipboardImageData(), mimeType: "image/png", isimage: true},
      ];
    }

    function test_paste(data) {
      if ( ! data.isimage) {
        OxideTestingUtils.copyToClipboard(data.mimeType, data.content);
      } else {
        OxideTestingUtils.copyImageToClipboard()
      }

      webView.url = "http://testsuite/tst_Clipboard.html";

      verify(webView.waitForLoadSucceeded(), "Timed out waiting for successful load");

      select_textarea_content();
      keyPress("v", Qt.ControlModifier)

      verify(webView.waitFor(function() { return expect_content(data.content); }));
    }

    function test_copy_data() {
      return [
        { content: "content", mimeType: "text/plain" },
      ];
    }

    function test_copy(data) {
      webView.url = "http://testsuite/tst_Clipboard.html";

      verify(webView.waitForLoadSucceeded(), "Timed out waiting for successful load");

      set_content(data.content)

      webView.waitFor(function() { return expect_content(data.content); });

      select_textarea_content();

      keyPress("c", Qt.ControlModifier)

      verify(webView.waitFor(function() {
        var current_content = OxideTestingUtils.copyFromClipboard(
          data.mimeType);
        return current_content === data.content
      }));
    }
  }
}
