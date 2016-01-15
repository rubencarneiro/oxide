import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.12
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: editingCapabilitiesSpy
    target: webView
    signalName: "editingCapabilitiesChanged"
  }

  TestCase {
    name: "WebView_editingCapabilities"
    when: windowShown

    function focus_textarea() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#textarea");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      compare(webView.getTestApi().evaluateCode(
          "return document.activeElement.id;", true),
          "textarea");
    }

    function unfocus_textarea() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#p");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      compare(webView.getTestApi().evaluateCode(
          "return document.activeElement.id;", true),
          "body");
    }

    function init() {
      webView.url = "http://testsuite/tst_WebView_editingCapabilities.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      Utils.clearClipboard();
      editingCapabilitiesSpy.clear();
    }

    function test_WebView_editingCapabilities_no_selection() {
      compare(webView.editingCapabilities, WebView.SelectAllCapability);
    }

    function test_WebView_editingCapabilities_page_selection() {
      webView.executeEditingCommand(WebView.EditingCommandSelectAll);
      editingCapabilitiesSpy.wait();
      compare(webView.editingCapabilities,
              WebView.CopyCapability | WebView.SelectAllCapability);

      editingCapabilitiesSpy.clear();
      webView.executeEditingCommand(WebView.EditingCommandCopy);
      compare(webView.editingCapabilities,
              WebView.CopyCapability | WebView.SelectAllCapability);
      compare(editingCapabilitiesSpy.count, 0);
    }

    function test_WebView_editingCapabilities_textarea_selection() {
      focus_textarea();
      editingCapabilitiesSpy.wait();
      compare(webView.editingCapabilities, WebView.SelectAllCapability);

      webView.executeEditingCommand(WebView.EditingCommandSelectAll);
      editingCapabilitiesSpy.wait();
      compare(webView.editingCapabilities,
              WebView.CutCapability | WebView.CopyCapability |
              WebView.EraseCapability | WebView.SelectAllCapability);

      webView.executeEditingCommand(WebView.EditingCommandCopy);
      editingCapabilitiesSpy.wait();
      compare(webView.editingCapabilities,
              WebView.CutCapability | WebView.CopyCapability |
              WebView.PasteCapability | WebView.EraseCapability |
              WebView.SelectAllCapability);

      webView.executeEditingCommand(WebView.EditingCommandCut);
      editingCapabilitiesSpy.wait();
      compare(webView.editingCapabilities,
              WebView.PasteCapability | WebView.SelectAllCapability);

      webView.executeEditingCommand(WebView.EditingCommandUndo);
      // FIXME: https://launchpad.net/bugs/1524288
      /*editingCapabilitiesSpy.wait();
      compare(webView.editingCapabilities,
              WebView.CutCapability | WebView.CopyCapability |
              WebView.PasteCapability | WebView.EraseCapability |
              WebView.SelectAllCapability);*/

      webView.executeEditingCommand(WebView.EditingCommandRedo);
      // FIXME: https://launchpad.net/bugs/1524288
      /*editingCapabilitiesSpy.wait();
      compare(webView.editingCapabilities,
              WebView.PasteCapability | WebView.SelectAllCapability);*/

      webView.executeEditingCommand(WebView.EditingCommandPaste);
      compare(webView.editingCapabilities,
              WebView.PasteCapability | WebView.SelectAllCapability);

      webView.executeEditingCommand(WebView.EditingCommandSelectAll);
      editingCapabilitiesSpy.wait();
      compare(webView.editingCapabilities,
              WebView.CutCapability | WebView.CopyCapability |
              WebView.PasteCapability | WebView.EraseCapability |
              WebView.SelectAllCapability);

      webView.executeEditingCommand(WebView.EditingCommandErase);
      // FIXME: https://launchpad.net/bugs/1524288
      /*editingCapabilitiesSpy.wait();
      compare(webView.editingCapabilities,
              WebView.PasteCapability | WebView.SelectAllCapability);*/
    }

    function test_WebView_editingCapabilities_clipboard_data_changed() {
      focus_textarea();
      compare(webView.editingCapabilities, WebView.SelectAllCapability);

      Utils.copyToClipboard("text/plain", "foo bar baz");
      editingCapabilitiesSpy.wait();
      compare(webView.editingCapabilities,
              WebView.PasteCapability | WebView.SelectAllCapability);

      Utils.clearClipboard();
      editingCapabilitiesSpy.wait();
      compare(webView.editingCapabilities, WebView.SelectAllCapability);
    }
  }
}
