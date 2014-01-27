import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property var currentPopupMenu: null

  popupMenu: Item {
    id: popup
    property var popupModel: model

    function getView() { return data[0]; }

    Component.onCompleted: {
      WebView.view.currentPopupMenu = popup;
    }

    ListView {
      model: popupModel.items
      delegate: Item {
        property string text: model.text
        property string group: model.group
        property int index: model.index
        property bool enabled: model.enabled
        property bool selected: model.selected
        property bool isSeparator: model.isSeparator
      }
    }
  }

  function waitForPopupMenu() {
    return waitFor(function() { return currentPopupMenu != null; });
  }

  TestCase {
    id: test
    name: "WebView_popupMenu_single"
    when: windowShown

    function init() {
      webView.currentPopupMenu = null;

      webView.url = "http://localhost:8080/tst_WebView_popupMenu_single.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView.getTestApi().getBoundingClientRectForSelector("#test");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      verify(webView.waitForPopupMenu(),
             "Timed out waiting for popup to show");
    }

    function test_WebView_popupMenu_single1_model() {
      verify(webView.currentPopupMenu.popupModel, "Should have a model");

      // Check model.elementRect
      var r = webView.getTestApi().getBoundingClientRectForSelector("#test");
      compare(webView.currentPopupMenu.popupModel.elementRect.x, r.x,
              "model.elementRect.x is wrong");
      compare(webView.currentPopupMenu.popupModel.elementRect.y, r.y,
              "model.elementRect.y is wrong");
      compare(webView.currentPopupMenu.popupModel.elementRect.width, r.width,
              "model.elementRect.width is wrong");
      compare(webView.currentPopupMenu.popupModel.elementRect.height, r.height,
              "model.elementRect.height is wrong");

      compare(webView.currentPopupMenu.popupModel.allowMultiSelect, false,
              "The select element doesn't allow multi-select");

      var data = [
        { text: "    Test 1", group: "Group A", enabled: true, selected: false, isSeparator: false },
        { text: "    Test 2", group: "Group A", enabled: true, selected: true, isSeparator: false },
        { text: "    Test 3", group: "Group B", enabled: true, selected: false, isSeparator: false },
        { text: "    Test 4", group: "Group B", enabled: false, selected: false, isSeparator: false },
        { text: "    Test 5", group: "Group C", enabled: false, selected: false, isSeparator: false },
        { text: "    Test 6", group: "Group C", enabled: false, selected: false, isSeparator: false },
      ];

      compare(webView.currentPopupMenu.getView().count, data.length,
              "Unexpected number of items in ListView");
      for (var i = 0; i < webView.currentPopupMenu.getView().count; ++i) {
        var item = webView.currentPopupMenu.getView().contentItem.children[i];
        compare(item.text, data[i].text);
        compare(item.group, data[i].group);
        compare(item.enabled, data[i].enabled);
        compare(item.selected, data[i].selected);
        compare(item.isSeparator, data[i].isSeparator);
        compare(item.index, i);
      }

      webView.currentPopupMenu.popupModel.cancel();
    }

    function test_WebView_popupMenu_single2_cancel() {
      webView.currentPopupMenu.popupModel.items.select(0);

      for (var i = 0; i < webView.currentPopupMenu.getView().count; ++i) {
        compare(webView.currentPopupMenu.getView().contentItem.children[i].selected,
                i == 0 ? true : false);
      }

      webView.currentPopupMenu.popupModel.cancel();

      var r = webView.getTestApi().evaluateCode(
          "document.querySelector(\"#test\").selectedIndex");
      compare(r, 1, "selectedIndex should not have changed");
    }

    function test_WebView_popupMenu_single3_accept() {
      webView.currentPopupMenu.popupModel.items.select(0);

      for (var i = 0; i < webView.currentPopupMenu.getView().count; ++i) {
        compare(webView.currentPopupMenu.getView().contentItem.children[i].selected,
                i == 0 ? true : false);
      }

      webView.currentPopupMenu.popupModel.accept();

      var r = webView.getTestApi().evaluateCode(
          "document.querySelector(\"#test\").selectedIndex");
      compare(r, 0, "selectedIndex should have changed");
    }

    function test_WebView_popupMenu_single4_invalidSelect() {
      webView.currentPopupMenu.popupModel.items.select(-1);
      webView.currentPopupMenu.popupModel.items.select(4);

      for (var i = 0; i < webView.currentPopupMenu.getView().count; ++i) {
        compare(webView.currentPopupMenu.getView().contentItem.children[i].selected,
                i == 1 ? true : false);
      }

      webView.currentPopupMenu.popupModel.cancel();
    }
  }
}
