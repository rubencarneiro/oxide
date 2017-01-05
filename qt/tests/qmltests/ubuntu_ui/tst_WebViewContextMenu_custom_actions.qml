import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.19
import Oxide.Ubuntu 1.0
import Oxide.testsupport 1.0
import Ubuntu.Components 1.3

UbuntuTestWebView {
  id: webView
  width: 800
  height: 600

  objectName: "webView"

  Action {
    id: customAction
    text: "Custom action"

    property int triggeredCount: 0
    onTriggered: ++triggeredCount
  }

  Action {
    id: disabledAction
    text: "Disabled action"
    enabled: false
  }

  Action {
    id: invisibleAction
    text: "Invisible action"
    visible: false
  }

  TestCase {
    id: test
    name: "WebViewContextMenu_custom_actions"
    when: windowShown

    function getContextMenu() {
      return TestSupport.findItemInScene(TestWindow.rootItem, "webView_WebContextMenu");
    }

    function waitForContextMenu() {
      var r = TestUtils.waitFor(function() { return !!getContextMenu() && getContextMenu().visible; });
      // This seems to be required to ensure that the menu is laid out
      TestSupport.wait(50);
      return r;
    }

    function waitForContextMenuToClose() {
      return TestUtils.waitFor(function() { return !getContextMenu(); });
    }

    function dismissContextMenu() {
      if (!getContextMenu()) {
        return;
      }

      mouseClick(webView, 1, 1);
      verify(waitForContextMenuToClose());
    }

    function getContextMenuEntryAtIndex(index) {
      return TestSupport.findItemInScene(getContextMenu(), "webView_WebContextMenu_item_" + index);
    }

    function verifyClipboardContents(type, value, msg) {
      verify(TestUtils.waitFor(function() { return ClipboardTestUtils.getFromClipboard(type) == value; }), msg);
    }

    function init() {
      ClipboardTestUtils.clearClipboard();
      dismissContextMenu();
      customAction.triggeredCount = 0;
      webView.url = "http://testsuite/tst_WebViewContextMenu.html";
      verify(webView.waitForLoadSucceeded());
    }

    function test_WebViewContextMenu_custom_actions1_remove_all() {
      function contextMenuOpeningHandler(params, menu) {
        menu.removeAll();
      }

      webView.contextMenuOpening.connect(contextMenuOpeningHandler);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#imagelink");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);

      TestSupport.wait(500);
      verify(!getContextMenu());

      webView.contextMenuOpening.disconnect(contextMenuOpeningHandler);
    }

    function test_WebViewContextMenu_custom_actions2_add_visible() {
      function contextMenuOpeningHandler(params, menu) {
        menu.appendAction(customAction);
      }

      webView.contextMenuOpening.connect(contextMenuOpeningHandler);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#canvas");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(2);
      verify(entry.visible);
      compare(entry.action, customAction);
      compare(entry.children[0].text, "Custom action");

      webView.contextMenuOpening.disconnect(contextMenuOpeningHandler);
    }

    function test_WebViewContextMenu_custom_actions3_trigger_custom_action() {
      function contextMenuOpeningHandler(params, menu) {
        menu.insertActionAt(1, customAction);
      }

      webView.contextMenuOpening.connect(contextMenuOpeningHandler);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#editable");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(1);
      mouseClick(entry);
      verify(waitForContextMenuToClose());

      compare(customAction.triggeredCount, 1);

      webView.contextMenuOpening.disconnect(contextMenuOpeningHandler);
    }

    function test_WebViewContextMenu_custom_actions4_add_disabled() {
      function contextMenuOpeningHandler(params, menu) {
        menu.appendAction(disabledAction);
      }

      webView.contextMenuOpening.connect(contextMenuOpeningHandler);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#image1");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(3);
      // We don't actually support disabled actions - we just make them invisible
      verify(!entry.visible);
      compare(entry.action, disabledAction);

      webView.contextMenuOpening.disconnect(contextMenuOpeningHandler);
    }

    function test_WebViewContextMenu_custom_actions4_add_invisible() {
      function contextMenuOpeningHandler(params, menu) {
        menu.appendAction(invisibleAction);
      }

      webView.contextMenuOpening.connect(contextMenuOpeningHandler);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#image1");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(3);
      // We don't actually support disabled actions - we just make them invisible
      verify(!entry.visible);
      compare(entry.action, invisibleAction);

      webView.contextMenuOpening.disconnect(contextMenuOpeningHandler);
    }

    function test_WebViewContextMenu_custom_actions5_move_stock_item() {
      function contextMenuOpeningHandler(params, menu) {
        var item = menu.items[0];
        menu.removeItem(item);
        menu.appendItem(item);
      }

      webView.contextMenuOpening.connect(contextMenuOpeningHandler);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#image1");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      webView.contextMenuOpening.disconnect(contextMenuOpeningHandler);

      var entry = getContextMenuEntryAtIndex(2);
      verify(entry.visible);
      mouseClick(entry);

      verifyClipboardContents("text/plain", "http://testsuite/cof.svg");
    }

    function test_WebViewContextMenu_custom_actions6_change_stock_item_text() {
      function contextMenuOpeningHandler(params, menu) {
        menu.items[0].action.text = "Foo";
      }

      webView.contextMenuOpening.connect(contextMenuOpeningHandler);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#image1");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      webView.contextMenuOpening.disconnect(contextMenuOpeningHandler);

      var entry = getContextMenuEntryAtIndex(0);
      verify(entry.visible);
      compare(entry.children[0].text, "Foo");
    }

    function test_WebViewContextMenu_custom_actions7_disable_stock_item() {
      function contextMenuOpeningHandler(params, menu) {
        menu.items[1].action.enabled = false;
      }

      webView.contextMenuOpening.connect(contextMenuOpeningHandler);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#image2");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      webView.contextMenuOpening.disconnect(contextMenuOpeningHandler);

      var entry = getContextMenuEntryAtIndex(1);
      verify(!entry.visible);
    }

    function test_WebViewContextMenu_custom_actions8_hide_stock_item() {
      function contextMenuOpeningHandler(params, menu) {
        menu.items[2].action.enabled = false;
      }

      webView.contextMenuOpening.connect(contextMenuOpeningHandler);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#imagelink");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      webView.contextMenuOpening.disconnect(contextMenuOpeningHandler);

      var entry = getContextMenuEntryAtIndex(2);
      verify(!entry.visible);
    }
  }
}
