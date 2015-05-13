import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView

  focus: true

  width: 200
  height: 200

  function expect_content(expected, rangemax) {
    var result = webView.getTestApi().evaluateCode(
        "return document.querySelector('#content').value", true);
    if (!rangemax) {
      rangemax = -1
    }
    return String(result).substr(0, rangemax) === expected.substr(0, rangemax);
  }

  function expect_has_file(expected) {
    var result = webView.getTestApi().evaluateCode(
        "return document.querySelector('#has_file').innerHTML", true);
    return (result === expected);
  }

  function expect_mime_type(expected) {
    var result = webView.getTestApi().evaluateCode(
        "return document.querySelector('#mime_type').innerHTML", true);
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

      if ( ! data.isimage) {
         select_textarea_content();
      }
      keyPress("v", Qt.ControlModifier)

      verify(webView.waitFor(function() { return expect_mime_type(data.mimeType); }));
      verify(webView.waitFor(function() { return expect_has_file(String(data.isimage)); }));

      /**
       The image we get from QImage and the one we pasted are slightly different
       but overall is the same image. QImage does some "processing" on the raw image content
       that slightly alters it and make it hard to have an exact match.
       */
      verify(webView.waitFor(function() { return expect_content(data.content, 41); }));
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
