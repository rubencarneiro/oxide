import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.8
import com.canonical.Oxide.Testing 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  TestCase {
    name: "WebView_executeEditingCommand"
    when: windowShown

    function compare_textarea_value(expected) {
      compare(webView.getTestApi().evaluateCode(
          "document.querySelector(\"#textarea\").value"), expected);
    }

    function get_selectionStart() {
      return webView.getTestApi().evaluateCode(
          "document.querySelector(\"#textarea\").selectionStart");
    }

    function get_selectionEnd() {
      return webView.getTestApi().evaluateCode(
          "document.querySelector(\"#textarea\").selectionEnd");
    }

    function init() {
      webView.url = "http://testsuite/tst_WebView_executeEditingCommand.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compare_textarea_value("lorem ipsum");
    }

    function test_WebView_executeEditingCommand_undo_redo() {
      webView.executeEditingCommand(WebView.EditingCommandSelectAll);
      keyClick(Qt.Key_Backspace);
      compare_textarea_value("");
      webView.executeEditingCommand(WebView.EditingCommandUndo);
      compare_textarea_value("lorem ipsum");
      webView.executeEditingCommand(WebView.EditingCommandRedo);
      compare_textarea_value("");
    }

    function test_WebView_executeEditingCommand_cut() {
      webView.executeEditingCommand(WebView.EditingCommandSelectAll);
      Utils.clearClipboard();
      webView.executeEditingCommand(WebView.EditingCommandCut);
      compare_textarea_value("");
      compare(Utils.getFromClipboard("text/plain"), "lorem ipsum");
    }

    function test_WebView_executeEditingCommand_copy() {
      webView.executeEditingCommand(WebView.EditingCommandSelectAll);
      Utils.clearClipboard();
      webView.executeEditingCommand(WebView.EditingCommandCopy);
      compare_textarea_value("lorem ipsum");
      compare(Utils.getFromClipboard("text/plain"), "lorem ipsum");
    }

    function test_WebView_executeEditingCommand_paste() {
      webView.executeEditingCommand(WebView.EditingCommandSelectAll);
      Utils.copyToClipboard("text/plain", "foo bar baz");
      webView.executeEditingCommand(WebView.EditingCommandPaste);
      compare_textarea_value("foo bar baz");
    }

    function test_WebView_executeEditingCommand_erase() {
      webView.executeEditingCommand(WebView.EditingCommandSelectAll);
      webView.executeEditingCommand(WebView.EditingCommandErase);
      compare_textarea_value("");
    }

    function test_WebView_executeEditingCommand_selectAll() {
      compare(get_selectionStart(), get_selectionEnd());
      webView.executeEditingCommand(WebView.EditingCommandSelectAll);
      compare(get_selectionStart(), 0);
      compare(get_selectionEnd(), "lorem ipsum".length);
    }
  }
}
