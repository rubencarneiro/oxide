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

  TestCase {
    id: test
    name: "WebViewPopupMenu"
    when: windowShown

    function getPopupMenu() {
      return TestSupport.findItemInScene(TestWindow.rootItem, "webView_WebPopupMenu");
    }

    function waitForPopupMenuToClose() {
      return TestUtils.waitFor(function() { return !getPopupMenu(); });
    }

    function waitForPopupMenu() {
      return TestUtils.waitFor(function() { return getPopupMenu(); });
    }

    function dismissPopupMenu() {
      if (!getPopupMenu()) {
        return;
      }

      mouseClick(webView, 1, 1);
      verify(waitForPopupMenuToClose());
    }

    function getPopupMenuEntryAtIndex(index) {
      return TestSupport.findItemInScene(getPopupMenu(), "webView_WebPopupMenu_item" + index);
    }

    function getSelectedIndex(selector) {
      return webView.getTestApi().evaluateCode("document.querySelector(\"" + selector + "\").selectedIndex");
    }

    function init() {
      dismissPopupMenu();
      webView.url = "http://testsuite/tst_WebViewPopupMenu.html";
      verify(webView.waitForLoadSucceeded());
    }

    function test_WebViewPopupMenu1_contents_data() {
      return [
        { selector: "#select1",
          items: [
            { type: "header", text: "Group A" },
            { type: "item", text: "    Test 1", enabled: true },
            { type: "item", text: "    Test 2", enabled: true },
            { type: "header", text: "Group B" },
            { type: "item", text: "    Test 3", enabled: true },
            { type: "item", text: "    Test 4", enabled: false },
            { type: "header", text: "Group C" },
            { type: "item", text: "    Test 5", enabled: false },
            { type: "item", text: "    Test 6", enabled: false }
          ]
        },
        { selector: "#select2",
          items: [
            { type: "item", text: "Test 1", enabled: true },
            { type: "item", text: "Test 2", enabled: true },
            { type: "item", text: "Test 3", enabled: true },
            { type: "item", text: "Test 4", enabled: false },
            { type: "item", text: "Test 5", enabled: true },
            { type: "item", text: "Test 6", enabled: true }
          ]
        }
      ];
    }

    function test_WebViewPopupMenu1_contents(data) {
      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      verify(waitForPopupMenu());

      var listView = TestSupport.findItemInScene(getPopupMenu(), "webView_WebPopupMenu_ListView");
      var items = [];
      for (var i in listView.children[0].children) {
        var item = listView.children[0].children[i];
        if (item.z == 0) {
          // Ignore the highlight item. This isn't a great way to detect this
          continue;
        }
        items.push(listView.children[0].children[i]);
      }
      items.sort(function(a, b) { return a.y - b.y; });

      compare(items.length, data.items.length);
      for (var i in items) {
        if (data.items[i].type == "item") {
          compare(items[i].listItemData[0].title.text, data.items[i].text);
          compare(items[i].enabled, data.items[i].enabled);
        } else if (data.items[i].type == "header") {
          compare(items[i].text, data.items[i].text);
        } else {
          fail();
        }
      }
    }

    function test_WebViewPopupMenu2_activate_disabled_data() {
      return [
        { selector: "#select1", index: 5, selectedIndex: 1 },
        { selector: "#select2", index: 3, selectedIndex: 2 }
      ];
    }

    function test_WebViewPopupMenu2_activate_disabled(data) {
      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      verify(waitForPopupMenu());

      var item = getPopupMenuEntryAtIndex(data.index);
      mouseClick(item);

      TestSupport.wait(100);
      verify(getPopupMenu());

      compare(getSelectedIndex(data.selector), data.selectedIndex);
    }

    function test_WebViewPopupMenu3_dismiss() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#select1");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      verify(waitForPopupMenu());

      dismissPopupMenu();
      verify(!getPopupMenu());

      compare(getSelectedIndex("#select1"), 1);
    }

    function test_WebViewPopupMenu4_activate_enabled() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#select2");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      verify(waitForPopupMenu());

      var item = getPopupMenuEntryAtIndex(0);
      mouseClick(item);

      verify(waitForPopupMenuToClose());

      compare(getSelectedIndex("#select2"), 0);
    }
  }
}
