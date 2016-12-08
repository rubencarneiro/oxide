import QtQuick 2.0
import QtTest 1.0
import Oxide.testsupport 1.0
import Oxide.Ubuntu 1.0
import Ubuntu.Components 1.3

UbuntuTestWebView {
  id: webView

  objectName: "webView"

  Component {
    id: menuFactory
    UbuntuWebContextMenu {}
  }

  Component {
    id: menuItemFactory
    UbuntuWebContextMenuItem {
      action: Action {}
    }
  }

  Component {
    id: actionFactory
    Action {}
  }

  ActionList {
    id: actionList

    Action {
      id: action1
    }

    Action {
      id: action2
    }
  }

  SignalSpy {
    id: spy
  }

  SignalSpy {
    id: spy2
  }

  TestCase {
    id: test
    name: "UbuntuWebContextMenu"
    when: windowShown

    function wrapCallbackTestSequence(callback) {
      var testCompleted = false;
      function runTest() {
        try {
          callback.apply(null, arguments);
        } catch(e) {
          fail("Callback test sequence threw exception: " + e);
        } finally {
          testCompleted = true;
        }
      }

      runTest.wait = function(timeout) {
        timeout = timeout || 5000;
        return TestUtils.waitFor(function() { return testCompleted; }, timeout);
      }

      return runTest;
    }

    function getContextMenu() {
      return TestSupport.findItemInScene(TestWindow.rootItem, "webView_WebContextMenu");
    }

    function waitForContextMenuToClose() {
      return TestUtils.waitFor(function() { return !getContextMenu(); });
    }

    function dismissContextMenu() {
      if (!getContextMenu()) {
        return;
      }

      mouseClick(webView, 1, 1);
      verify(TestUtils.waitFor(function() { return !getContextMenu(); }));
    }

    function init() {
      dismissContextMenu();
      spy.target = null;
      spy.signalName = "";
      spy.clear();
      spy2.target = null;
      spy2.signalName = "";
      spy2.clear();
      webView.clearLoadEventCounters();
    }

    function test_UbuntuWebContextMenu01_default() {
      var menu = menuFactory.createObject(null, {});
      verify(menu.isEmpty);
      compare(menu.items.length, 0);
    }

    function test_UbuntuWebContextMenu02_isEmpty() {
      var menu = menuFactory.createObject(null, {});
      spy.target = menu;
      spy.signalName = "isEmptyChanged";

      verify(menu.isEmpty);

      var item = menuItemFactory.createObject(null, {});

      menu.appendItem(item);
      compare(spy.count, 1);
      verify(!menu.isEmpty);

      menu.removeItem(item);
      compare(spy.count, 2);
      verify(menu.isEmpty);
    }

    function test_UbuntuWebContextMenu03_indexOfItem() {
      var menu = menuFactory.createObject(null, {});

      var item1 = menuItemFactory.createObject(null, {});
      var item2 = menuItemFactory.createObject(null, {});
      var item3 = menuItemFactory.createObject(null, {});
      var item4 = menuItemFactory.createObject(null, {});
      var item5 = menuItemFactory.createObject(null, {});
      var item6 = menuItemFactory.createObject(null, {});

      menu.appendItem(item1);
      menu.appendItem(item2);
      menu.appendItem(item3);
      menu.appendItem(item4);
      menu.appendItem(item5);

      compare(menu.indexOfItem(item2), 1);
      compare(menu.indexOfItem(item5), 4);

      // Verify that calling indexOfItem with an item not in the menu works
      // correctly
      compare(menu.indexOfItem(item6), -1);

      // Verify that calling indexOfItem with a null item works correctly
      compare(menu.indexOfItem(null), -1);
    }

    function test_UbuntuWebContextMenu04_indexOfStockAction() {
      webView.url = "http://testsuite/tst_UbuntuWebContextMenu.html";
      verify(webView.waitForLoadSucceeded());

      function contextMenuOpeningHandler(params, menu) {
        compare(menu.indexOfStockAction(UbuntuWebContextMenuItem.ActionCopyLinkLocation), 0);
        compare(menu.indexOfStockAction(UbuntuWebContextMenuItem.ActionCopyMediaLocation), 2);
        compare(menu.indexOfStockAction(UbuntuWebContextMenuItem.ActionCopyImage), 4);

        // Verify that calling indexOfStockAction with an action not present in
        // the menu works correctly
        compare(menu.indexOfStockAction(UbuntuWebContextMenuItem.NoAction), -1);
        compare(menu.indexOfStockAction(UbuntuWebContextMenuItem.ActionUndo), -1);
      }

      var callback = wrapCallbackTestSequence(contextMenuOpeningHandler);
      webView.contextMenuOpening.connect(callback);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#imagelink");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(callback.wait());

      webView.contextMenuOpening.disconnect(callback);
    }

    function test_UbuntuWebContextMenu05_appendItem() {
      var menu = menuFactory.createObject(null, {});

      var item1 = menuItemFactory.createObject(null, {});
      var item2 = menuItemFactory.createObject(test, {});
      var item3 = menuItemFactory.createObject(webView, {});
      var item4 = menuItemFactory.createObject(null, {});

      spy.target = menu;
      spy.signalName = "itemsChanged";

      menu.appendItem(item1);
      compare(spy.count, 1);
      compare(menu.items.length, 1);
      // All items are reparented
      compare(TestSupport.qObjectParent(item1), menu);

      menu.appendItem(item2);
      compare(spy.count, 2);
      compare(menu.items.length, 2);
      // Menu should take ownership of application owner item
      compare(TestSupport.qObjectParent(item2), menu);

      menu.appendItem(item3);
      compare(spy.count, 3);
      compare(menu.items.length, 3);
      compare(TestSupport.qObjectParent(item3), menu);

      menu.appendItem(item4);
      compare(spy.count, 4);
      compare(menu.items.length, 4);
      compare(TestSupport.qObjectParent(item4), menu);

      // Calling appendItem with a null item shouldn't crash or make changes
      menu.appendItem(null);
      compare(spy.count, 4);
      compare(menu.items.length, 4);

      // Calling appendItem with an item already in a menu should have no effect
      menu.appendItem(item4);
      compare(spy.count, 4);
      compare(menu.items.length, 4);

      compare(menu.items[0], item1);
      compare(menu.items[1], item2);
      compare(menu.items[2], item3);
      compare(menu.items[3], item4);
    }

    function test_UbuntuWebContextMenu06_appendItemToSection() {
      webView.url = "http://testsuite/tst_UbuntuWebContextMenu.html";
      verify(webView.waitForLoadSucceeded());

      function contextMenuOpeningHandler(params, menu) {
        spy.target = menu;
        spy.signalName = "itemsChanged";

        spy2.signalName = "sectionChanged";

        var item1 = menuItemFactory.createObject(null, {});
        var item2 = menuItemFactory.createObject(null, {});
        var item3 = menuItemFactory.createObject(null, {});
        var item4 = menuItemFactory.createObject(null, {});

        spy2.target = item1;
        compare(menu.appendItemToSection(UbuntuWebContextMenuItem.SectionLink, item1), 2);
        compare(spy.count, 1);
        compare(menu.items.length, 6);
        // Item should be reparented to the menu
        compare(TestSupport.qObjectParent(item1), menu);
        // The item's section should be updated
        compare(spy2.count, 1);
        compare(item1.section, UbuntuWebContextMenuItem.SectionLink);

        spy2.target = item2;
        spy2.clear();
        compare(menu.appendItemToSection(UbuntuWebContextMenuItem.SectionMedia, item2), 6);
        compare(spy.count, 2);
        compare(menu.items.length, 7);
        compare(TestSupport.qObjectParent(item2), menu);
        compare(spy2.count, 1);
        compare(item2.section, UbuntuWebContextMenuItem.SectionMedia);

        spy2.target = item3;
        spy2.clear();
        compare(menu.appendItemToSection(UbuntuWebContextMenuItem.SectionLink, item3), 3);
        compare(spy.count, 3);
        compare(menu.items.length, 8);
        compare(TestSupport.qObjectParent(item3), menu);
        compare(spy2.count, 1);
        compare(item3.section, UbuntuWebContextMenuItem.SectionLink);

        spy2.target = item4;
        spy2.clear();
        // Verify that specifying a non-existant section behaves as expected
        compare(menu.appendItemToSection(UbuntuWebContextMenuItem.SectionEditing, item4), -1);
        compare(spy.count, 3);
        compare(menu.items.length, 8);
        // The item shouldn't have been reparented ot had its section changed
        compare(TestSupport.qObjectParent(item4), null);
        compare(spy2.count, 0);
        compare(item4.section, UbuntuWebContextMenuItem.NoSection);

        compare(menu.items[2], item1);
        compare(menu.items[7], item2);
        compare(menu.items[3], item3);
      }

      var callback = wrapCallbackTestSequence(contextMenuOpeningHandler);
      webView.contextMenuOpening.connect(callback);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#imagelink");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(callback.wait());

      webView.contextMenuOpening.disconnect(callback);
    }

    function test_UbuntuWebContextMenu07_appendAction() {
      var menu = menuFactory.createObject(null, {});

      var action3 = actionFactory.createObject(null, {});
      var action4 = actionFactory.createObject(null, {});

      spy.target = menu;
      spy.signalName = "itemsChanged";

      menu.appendAction(action1);
      compare(spy.count, 1);
      compare(menu.items.length, 1);
      // An application owned action shouldn't be reparented
      compare(TestSupport.qObjectParent(action1), actionList);
      compare(menu.items[0].action, action1);
      // The created item should be owned by the menu
      compare(TestSupport.qObjectParent(menu.items[0]), menu);
      // The created item should have no stockAction or section
      compare(menu.items[0].stockAction, UbuntuWebContextMenuItem.NoAction);
      compare(menu.items[0].section, UbuntuWebContextMenuItem.NoSection);

      menu.appendAction(action2);
      compare(spy.count, 2);
      compare(menu.items.length, 2);
      compare(TestSupport.qObjectParent(action2), actionList);
      compare(menu.items[1].action, action2);
      compare(TestSupport.qObjectParent(menu.items[1]), menu);
      compare(menu.items[1].stockAction, UbuntuWebContextMenuItem.NoAction);
      compare(menu.items[1].section, UbuntuWebContextMenuItem.NoSection);

      menu.appendAction(action3);
      compare(spy.count, 3);
      compare(menu.items.length, 3);
      // An unowned action should be reparented to the created item
      compare(TestSupport.qObjectParent(action3), menu.items[2]);
      compare(menu.items[2].action, action3);
      compare(TestSupport.qObjectParent(menu.items[2]), menu);
      compare(menu.items[2].stockAction, UbuntuWebContextMenuItem.NoAction);
      compare(menu.items[2].section, UbuntuWebContextMenuItem.NoSection);

      menu.appendAction(action4);
      compare(spy.count, 4);
      compare(menu.items.length, 4);
      compare(TestSupport.qObjectParent(action4), menu.items[3]);
      compare(menu.items[3].action, action4);
      compare(TestSupport.qObjectParent(menu.items[3]), menu);
      compare(menu.items[3].stockAction, UbuntuWebContextMenuItem.NoAction);
      compare(menu.items[3].section, UbuntuWebContextMenuItem.NoSection);

      // Verify that calling appendAction with no action behaves as expected
      menu.appendAction(null);
      compare(spy.count, 4);
      compare(menu.items.length, 4);

      compare(menu.items[0].action, action1);
      compare(menu.items[1].action, action2);
      compare(menu.items[2].action, action3);
      compare(menu.items[3].action, action4);
    }

    function test_UbuntuWebContextMenu08_appendActionToSection() {
      webView.url = "http://testsuite/tst_UbuntuWebContextMenu.html";
      verify(webView.waitForLoadSucceeded());

      function contextMenuOpeningHandler(params, menu) {
        spy.target = menu;
        spy.signalName = "itemsChanged";

        var action3 = actionFactory.createObject(null, {});
        var action4 = actionFactory.createObject(null, {});

        compare(menu.appendActionToSection(UbuntuWebContextMenuItem.SectionLink, action1), 2);
        compare(spy.count, 1);
        compare(menu.items.length, 6);
        // An application owned action shouldn't be reparented
        compare(TestSupport.qObjectParent(action1), actionList);
        compare(menu.items[2].action, action1);
        // The created item should be owned by the menu
        compare(TestSupport.qObjectParent(menu.items[2]), menu);
        // The created item's section should match that in to which it was inserted
        compare(menu.items[2].section, UbuntuWebContextMenuItem.SectionLink);
        compare(menu.items[2].stockAction, UbuntuWebContextMenuItem.NoAction);

        compare(menu.appendActionToSection(UbuntuWebContextMenuItem.SectionMedia, action2), 6);
        compare(spy.count, 2);
        compare(menu.items.length, 7);
        compare(TestSupport.qObjectParent(action2), actionList);
        compare(menu.items[6].action, action2);
        compare(TestSupport.qObjectParent(menu.items[6]), menu);
        compare(menu.items[6].section, UbuntuWebContextMenuItem.SectionMedia);
        compare(menu.items[6].stockAction, UbuntuWebContextMenuItem.NoAction);

        compare(menu.appendActionToSection(UbuntuWebContextMenuItem.SectionMedia, action3), 7);
        compare(spy.count, 3);
        compare(menu.items.length, 8);
        // An unowned action should be reparented to the created item
        compare(TestSupport.qObjectParent(action3), menu.items[7]);
        compare(menu.items[7].action, action3);
        compare(TestSupport.qObjectParent(menu.items[7]), menu);
        compare(menu.items[7].section, UbuntuWebContextMenuItem.SectionMedia);
        compare(menu.items[7].stockAction, UbuntuWebContextMenuItem.NoAction);

        // Verify that specifying a non-existant section behaves as expected
        var helper = TestSupport.createQObjectTestHelper(action4);
        compare(menu.appendActionToSection(UbuntuWebContextMenuItem.SectionEditing, action4), -1);
        compare(spy.count, 3);
        compare(menu.items.length, 8);
        // The action shouldn't have been reparented or destroyed
        compare(TestSupport.qObjectParent(action4), null);
        verify(!helper.destroyed);

        compare(menu.items[2].action, action1);
        compare(menu.items[6].action, action2);
        compare(menu.items[7].action, action3);
      }

      var callback = wrapCallbackTestSequence(contextMenuOpeningHandler);
      webView.contextMenuOpening.connect(callback);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#imagelink");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(callback.wait());

      webView.contextMenuOpening.disconnect(callback);
    }

    function test_UbuntuWebContextMenu09_prependItem() {
      var menu = menuFactory.createObject(null, {});

      var item1 = menuItemFactory.createObject(null, {});
      var item2 = menuItemFactory.createObject(test, {});
      var item3 = menuItemFactory.createObject(webView, {});
      var item4 = menuItemFactory.createObject(null, {});

      spy.target = menu;
      spy.signalName = "itemsChanged";

      menu.prependItem(item1);
      compare(spy.count, 1);
      compare(menu.items.length, 1);
      // All items should be reparented to the menu
      compare(TestSupport.qObjectParent(item1), menu);

      menu.prependItem(item2);
      compare(spy.count, 2);
      compare(menu.items.length, 2);
      // All items should be reparented to the menu
      compare(TestSupport.qObjectParent(item2), menu);

      menu.prependItem(item3);
      compare(spy.count, 3);
      compare(menu.items.length, 3);
      compare(TestSupport.qObjectParent(item3), menu);

      menu.prependItem(item4);
      compare(spy.count, 4);
      compare(menu.items.length, 4);
      compare(TestSupport.qObjectParent(item4), menu);

      // Verify that calling prependItem with a null item behaves as expected
      menu.prependItem(null);
      compare(spy.count, 4);
      compare(menu.items.length, 4);

      // Verify that calling prependItem with an item that already belongs to
      // a menu behaves as expected
      menu.prependItem(item4);
      compare(spy.count, 4);
      compare(menu.items.length, 4);

      compare(menu.items[0], item4);
      compare(menu.items[1], item3);
      compare(menu.items[2], item2);
      compare(menu.items[3], item1);
    }

    function test_UbuntuWebContextMenu10_prependItemToSection() {
      webView.url = "http://testsuite/tst_UbuntuWebContextMenu.html";
      verify(webView.waitForLoadSucceeded());

      function contextMenuOpeningHandler(params, menu) {
        spy.target = menu;
        spy.signalName = "itemsChanged";

        spy2.signalName = "sectionChanged";

        var item1 = menuItemFactory.createObject(null, {});
        var item2 = menuItemFactory.createObject(null, {});
        var item3 = menuItemFactory.createObject(null, {});
        var item4 = menuItemFactory.createObject(null, {});

        spy2.target = item1;
        compare(menu.prependItemToSection(UbuntuWebContextMenuItem.SectionLink, item1), 0);
        compare(spy.count, 1);
        compare(menu.items.length, 6);
        // The item should be reparented to the menu
        compare(TestSupport.qObjectParent(item1), menu);
        // Verify that the item's section is updated
        compare(spy2.count, 1);
        compare(item1.section, UbuntuWebContextMenuItem.SectionLink);

        spy2.target = item2;
        spy2.clear();
        compare(menu.prependItemToSection(UbuntuWebContextMenuItem.SectionMedia, item2), 3);
        compare(spy.count, 2);
        compare(menu.items.length, 7);
        compare(TestSupport.qObjectParent(item2), menu);
        compare(spy2.count, 1);
        compare(item2.section, UbuntuWebContextMenuItem.SectionMedia);

        spy2.target = item3;
        spy2.clear();
        compare(menu.prependItemToSection(UbuntuWebContextMenuItem.SectionLink, item3), 0);
        compare(spy.count, 3);
        compare(menu.items.length, 8);
        compare(TestSupport.qObjectParent(item3), menu);
        compare(spy2.count, 1);
        compare(item3.section, UbuntuWebContextMenuItem.SectionLink);

        spy2.target = item4;
        spy2.clear();
        // Verify that calling prependItemToSection with a non-existant section
        // behaves as expected
        compare(menu.prependItemToSection(UbuntuWebContextMenuItem.SectionEditing, item4), -1);
        compare(spy.count, 3);
        compare(menu.items.length, 8);
        // The item shouldn't have been reparented and it's section shouldn't
        // be updated
        compare(TestSupport.qObjectParent(item4), null);
        compare(spy2.count, 0);
        compare(item4.section, UbuntuWebContextMenuItem.NoSection);

        compare(menu.items[1], item1);
        compare(menu.items[4], item2);
        compare(menu.items[0], item3);
      }

      var callback = wrapCallbackTestSequence(contextMenuOpeningHandler);
      webView.contextMenuOpening.connect(callback);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#imagelink");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(callback.wait());

      webView.contextMenuOpening.disconnect(callback);
    }

    function test_UbuntuWebContextMenu11_prependAction() {
      var menu = menuFactory.createObject(null, {});

      var action3 = actionFactory.createObject(null, {});
      var action4 = actionFactory.createObject(null, {});

      spy.target = menu;
      spy.signalName = "itemsChanged";

      menu.prependAction(action1);
      compare(spy.count, 1);
      compare(menu.items.length, 1);
      // An application owned action shouldn't be reparented
      compare(TestSupport.qObjectParent(action1), actionList);
      compare(menu.items[0].action, action1);
      // The created item should be owned by the menu
      compare(TestSupport.qObjectParent(menu.items[0]), menu);
      // The created item should have no section or stockAction
      compare(menu.items[0].stockAction, UbuntuWebContextMenuItem.NoAction);
      compare(menu.items[0].section, UbuntuWebContextMenuItem.NoSection);

      menu.prependAction(action2);
      compare(spy.count, 2);
      compare(menu.items.length, 2);
      compare(TestSupport.qObjectParent(action2), actionList);
      compare(menu.items[0].action, action2);
      compare(TestSupport.qObjectParent(menu.items[0]), menu);
      compare(menu.items[0].stockAction, UbuntuWebContextMenuItem.NoAction);
      compare(menu.items[0].section, UbuntuWebContextMenuItem.NoSection);

      menu.prependAction(action3);
      compare(spy.count, 3);
      compare(menu.items.length, 3);
      // An unowned action should be reparented to the menu
      compare(TestSupport.qObjectParent(action3), menu.items[0]);
      compare(menu.items[0].action, action3);
      compare(TestSupport.qObjectParent(menu.items[0]), menu);
      compare(menu.items[0].stockAction, UbuntuWebContextMenuItem.NoAction);
      compare(menu.items[0].section, UbuntuWebContextMenuItem.NoSection);

      menu.prependAction(action4);
      compare(spy.count, 4);
      compare(menu.items.length, 4);
      compare(TestSupport.qObjectParent(action4), menu.items[0]);
      compare(menu.items[0].action, action4);
      compare(TestSupport.qObjectParent(menu.items[0]), menu);
      compare(menu.items[0].stockAction, UbuntuWebContextMenuItem.NoAction);
      compare(menu.items[0].section, UbuntuWebContextMenuItem.NoSection);

      // Verify that calling appendAction with a null action behaves as expected
      menu.appendAction(null);
      compare(spy.count, 4);
      compare(menu.items.length, 4);

      compare(menu.items[0].action, action4);
      compare(menu.items[1].action, action3);
      compare(menu.items[2].action, action2);
      compare(menu.items[3].action, action1);
    }

    function test_UbuntuWebContextMenu12_prependActionToSection() {
      webView.url = "http://testsuite/tst_UbuntuWebContextMenu.html";
      verify(webView.waitForLoadSucceeded());

      function contextMenuOpeningHandler(params, menu) {
        spy.target = menu;
        spy.signalName = "itemsChanged";

        var action3 = actionFactory.createObject(null, {});
        var action4 = actionFactory.createObject(null, {});

        compare(menu.prependActionToSection(UbuntuWebContextMenuItem.SectionLink, action1), 0);
        compare(spy.count, 1);
        compare(menu.items.length, 6);
        // An application owned action shouldn't be reparented
        compare(TestSupport.qObjectParent(action1), actionList);
        compare(menu.items[0].action, action1);
        // The created item should be owned by the menu
        compare(TestSupport.qObjectParent(menu.items[0]), menu);
        // Verify that the section for the created item matches that in to
        // which it was inserted
        compare(menu.items[0].section, UbuntuWebContextMenuItem.SectionLink);
        compare(menu.items[0].stockAction, UbuntuWebContextMenuItem.NoAction);

        compare(menu.prependActionToSection(UbuntuWebContextMenuItem.SectionMedia, action2), 3);
        compare(spy.count, 2);
        compare(menu.items.length, 7);
        compare(TestSupport.qObjectParent(action2), actionList);
        compare(menu.items[3].action, action2);
        compare(TestSupport.qObjectParent(menu.items[3]), menu);
        compare(menu.items[3].section, UbuntuWebContextMenuItem.SectionMedia);
        compare(menu.items[3].stockAction, UbuntuWebContextMenuItem.NoAction);

        compare(menu.prependActionToSection(UbuntuWebContextMenuItem.SectionMedia, action3), 3);
        compare(spy.count, 3);
        compare(menu.items.length, 8);
        // An unowned action should be reparented to the created item
        compare(TestSupport.qObjectParent(action3), menu.items[3]);
        compare(menu.items[3].action, action3);
        compare(TestSupport.qObjectParent(menu.items[3]), menu);
        compare(menu.items[3].section, UbuntuWebContextMenuItem.SectionMedia);
        compare(menu.items[3].stockAction, UbuntuWebContextMenuItem.NoAction);

        var helper = TestSupport.createQObjectTestHelper(action4);
        // Verify that calling prependActionToSection with a non-existant
        // section behaves as expected
        compare(menu.prependActionToSection(UbuntuWebContextMenuItem.SectionEditing, action4), -1);
        compare(spy.count, 3);
        compare(menu.items.length, 8);
        // The action shouldn't be reparented or destroyed
        compare(TestSupport.qObjectParent(action4), null);
        verify(!helper.destroyed);

        compare(menu.items[0].action, action1);
        compare(menu.items[4].action, action2);
        compare(menu.items[3].action, action3);
      }

      var callback = wrapCallbackTestSequence(contextMenuOpeningHandler);
      webView.contextMenuOpening.connect(callback);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#imagelink");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(callback.wait());

      webView.contextMenuOpening.disconnect(callback);
    }

    function test_UbuntuWebContextMenu13_insertItemAt() {
      webView.url = "http://testsuite/tst_UbuntuWebContextMenu.html";
      verify(webView.waitForLoadSucceeded());

      function contextMenuOpeningHandler(params, menu) {
        spy.target = menu;
        spy.signalName = "itemsChanged";

        spy2.signalName = "sectionChanged";

        var item1 = menuItemFactory.createObject(null, {});
        var item2 = menuItemFactory.createObject(webView, {});
        var item3 = menuItemFactory.createObject(test, {});
        var item4 = menuItemFactory.createObject(null, {});
        var item5 = menuItemFactory.createObject(null, {});

        spy2.target = item1;
        menu.insertItemAt(0, item1);
        compare(spy.count, 1);
        compare(menu.items.length, 6);
        // All items should be reparented to the menu
        compare(TestSupport.qObjectParent(menu.items[0]), menu);
        // The item's section shouldn't be updated
        compare(spy2.count, 0);
        compare(item1.section, UbuntuWebContextMenuItem.NoSection);

        spy2.target = item2;
        spy2.clear();
        menu.insertItemAt(-10, item2);
        compare(spy.count, 2);
        compare(menu.items.length, 7);
        compare(TestSupport.qObjectParent(menu.items[0]), menu);
        compare(spy2.count, 0);
        compare(item1.section, UbuntuWebContextMenuItem.NoSection);

        spy2.target = item3;
        spy2.clear();
        menu.insertItemAt(5, item3);
        compare(spy.count, 3);
        compare(menu.items.length, 8);
        compare(TestSupport.qObjectParent(menu.items[5]), menu);
        compare(spy2.count, 1);
        // Verify that inserting an item in to the middle of a section
        // effectively makes it part of that section
        compare(item3.section, UbuntuWebContextMenuItem.SectionMedia);

        spy2.target = item4;
        spy2.clear();
        menu.insertItemAt(8, item4);
        compare(spy.count, 4);
        compare(menu.items.length, 9);
        compare(TestSupport.qObjectParent(menu.items[8]), menu);
        compare(spy2.count, 0);
        compare(item1.section, UbuntuWebContextMenuItem.NoSection);

        spy2.target = item5;
        spy2.clear();
        menu.insertItemAt(20, item5);
        compare(spy.count, 5);
        compare(menu.items.length, 10);
        compare(TestSupport.qObjectParent(menu.items[9]), menu);
        compare(spy2.count, 0);
        compare(item1.section, UbuntuWebContextMenuItem.NoSection);

        // Verify that calling insertItemAt with a null item behaves as expected
        menu.insertItemAt(5, null);
        compare(spy.count, 5);
        compare(menu.items.length, 10);

        // Verify that adding an item that is already part of a menu works as expected
        menu.insertItemAt(5, item1);
        compare(spy.count, 5);
        compare(menu.items.length, 10);

        compare(menu.items[0], item2);
        compare(menu.items[1], item1);
        compare(menu.items[5], item3);
        compare(menu.items[8], item4);
        compare(menu.items[9], item5);
      }

      var callback = wrapCallbackTestSequence(contextMenuOpeningHandler);
      webView.contextMenuOpening.connect(callback);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#imagelink");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(callback.wait());

      webView.contextMenuOpening.disconnect(callback);
    }

    function test_UbuntuWebContextMenu14_insertActionAt() {
      var menu = menuFactory.createObject(null, {});

      spy.target = menu;
      spy.signalName = "itemsChanged";

      var action3 = actionFactory.createObject(null, {});
      var action4 = actionFactory.createObject(null, {});
      var action5 = actionFactory.createObject(null, {});

      menu.insertActionAt(0, action1);
      compare(spy.count, 1);
      compare(menu.items.length, 1);
      // The created item should be owned by the menu
      compare(TestSupport.qObjectParent(menu.items[0]), menu);
      compare(menu.items[0].action, action1);
      // An application owned action shouldn't be reparented
      compare(TestSupport.qObjectParent(action1), actionList);
      compare(menu.items[0].stockAction, UbuntuWebContextMenuItem.NoAction);
      // The created item should have no section
      compare(menu.items[0].section, UbuntuWebContextMenuItem.NoSection);

      menu.insertActionAt(-10, action2);
      compare(spy.count, 2);
      compare(menu.items.length, 2);
      compare(TestSupport.qObjectParent(menu.items[0]), menu);
      compare(menu.items[0].action, action2);
      compare(TestSupport.qObjectParent(action2), actionList);
      compare(menu.items[0].stockAction, UbuntuWebContextMenuItem.NoAction);
      compare(menu.items[0].section, UbuntuWebContextMenuItem.NoSection);

      menu.insertActionAt(1, action3);
      compare(spy.count, 3);
      compare(menu.items.length, 3);
      compare(TestSupport.qObjectParent(menu.items[1]), menu);
      compare(menu.items[1].action, action3);
      // An unowned action should be reparented to the created item
      compare(TestSupport.qObjectParent(action3), menu.items[1]);
      compare(menu.items[1].stockAction, UbuntuWebContextMenuItem.NoAction);
      compare(menu.items[1].section, UbuntuWebContextMenuItem.NoSection);

      menu.insertActionAt(3, action4);
      compare(spy.count, 4);
      compare(menu.items.length, 4);
      compare(TestSupport.qObjectParent(menu.items[3]), menu);
      compare(TestSupport.qObjectParent(action4), menu.items[3]);
      compare(menu.items[3].stockAction, UbuntuWebContextMenuItem.NoAction);
      compare(menu.items[3].section, UbuntuWebContextMenuItem.NoSection);

      menu.insertActionAt(10, action5);
      compare(spy.count, 5);
      compare(menu.items.length, 5);
      compare(TestSupport.qObjectParent(menu.items[4]), menu);
      compare(TestSupport.qObjectParent(action5), menu.items[4]);
      compare(menu.items[4].stockAction, UbuntuWebContextMenuItem.NoAction);
      compare(menu.items[4].section, UbuntuWebContextMenuItem.NoSection);

      // Verify that calling insertActionAt with a null item behaves as expected
      menu.insertActionAt(4, null);
      compare(spy.count, 5);
      compare(menu.items.length, 5);

      compare(menu.items[0].action, action2);
      compare(menu.items[1].action, action3);
      compare(menu.items[2].action, action1);
      compare(menu.items[3].action, action4);
      compare(menu.items[4].action, action5);
    }

    function test_UbuntuWebContextMenu15_insertItemBefore() {
      webView.url = "http://testsuite/tst_UbuntuWebContextMenu.html";
      verify(webView.waitForLoadSucceeded());

      function contextMenuOpeningHandler(params, menu) {
        spy.target = menu;
        spy.signalName = "itemsChanged";

        spy2.signalName = "sectionChanged";

        var item1 = menuItemFactory.createObject(null, {});
        var item2 = menuItemFactory.createObject(null, {});

        spy2.target = item1;
        // Verify that calling insertItemBefore with a non-existant stock action
        // behaves as expected
        compare(menu.insertItemBefore(UbuntuWebContextMenuItem.ActionUndo, item1), -1);
        compare(spy.count, 0);
        compare(menu.items.length, 5);
        // The specified item shouldn't be reparented
        compare(TestSupport.qObjectParent(item1), null);
        compare(spy2.count, 0);

        compare(menu.insertItemBefore(UbuntuWebContextMenuItem.ActionSaveLink, item1), 1);
        compare(spy.count, 1);
        compare(menu.items.length, 6);
        // The item should be reparented to the menu
        compare(TestSupport.qObjectParent(item1), menu);
        // Item's inserted in to a section should have their section property
        // updated accordingly
        compare(item1.section, UbuntuWebContextMenuItem.SectionLink);
        compare(spy2.count, 1);

        spy2.target = item2;
        spy2.clear();
        compare(menu.insertItemBefore(UbuntuWebContextMenuItem.ActionCopyMediaLocation, item2), 3);
        compare(spy.count, 2);
        compare(menu.items.length, 7);
        compare(TestSupport.qObjectParent(item2), menu);
        // Verify that items not inserted in to a section don't have their
        // section property updated
        compare(item2.section, UbuntuWebContextMenuItem.NoSection);
        compare(spy2.count, 0);

        compare(menu.items[1], item1);
        compare(menu.items[3], item2);
      }

      var callback = wrapCallbackTestSequence(contextMenuOpeningHandler);
      webView.contextMenuOpening.connect(callback);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#imagelink");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(callback.wait());

      webView.contextMenuOpening.disconnect(callback);
    }

    function test_UbuntuWebContextMenu16_insertActionBefore() {
      webView.url = "http://testsuite/tst_UbuntuWebContextMenu.html";
      verify(webView.waitForLoadSucceeded());

      function contextMenuOpeningHandler(params, menu) {
        spy.target = menu;
        spy.signalName = "itemsChanged";

        var action3 = actionFactory.createObject(null, {});
        var helper3 = TestSupport.createQObjectTestHelper(action3);

        // Verify that calling insertActionBefore with a non-existant stock
        // action behaves as expected
        compare(menu.insertActionBefore(UbuntuWebContextMenuItem.ActionUndo, action3), -1);
        compare(spy.count, 0);
        compare(menu.items.length, 5);
        // The action shouldn't have been reparented or destroyed
        compare(TestSupport.qObjectParent(action3), null);
        verify(!helper3.destroyed);

        compare(menu.insertActionBefore(UbuntuWebContextMenuItem.ActionSaveLink, action3), 1);
        compare(spy.count, 1);
        compare(menu.items.length, 6);
        // The created item should be owned by the menu
        compare(TestSupport.qObjectParent(menu.items[1]), menu);
        compare(menu.items[1].action, action3);
        // The action should have been reparented to the created item
        compare(TestSupport.qObjectParent(action3), menu.items[1]);
        compare(menu.items[1].stockAction, UbuntuWebContextMenuItem.NoAction);
        // Verify that the section property is updated accordingly for items
        // inserted in to the middle of a section
        compare(menu.items[1].section, UbuntuWebContextMenuItem.SectionLink);

        compare(menu.insertActionBefore(UbuntuWebContextMenuItem.ActionCopyMediaLocation, action1), 3);
        compare(spy.count, 2);
        compare(menu.items.length, 7);
        compare(TestSupport.qObjectParent(menu.items[3]), menu);
        compare(menu.items[3].action, action1);
        // Application owned actions shouldn't be reparented
        compare(TestSupport.qObjectParent(action1), actionList);
        compare(menu.items[3].stockAction, UbuntuWebContextMenuItem.NoAction);
        // Items not inserted in to the middle of a section shouldn't have their
        // section property updated
        compare(menu.items[3].section, UbuntuWebContextMenuItem.NoSection);

        compare(menu.items[1].action, action3);
        compare(menu.items[3].action, action1);
      }

      var callback = wrapCallbackTestSequence(contextMenuOpeningHandler);
      webView.contextMenuOpening.connect(callback);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#imagelink");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(callback.wait());

      webView.contextMenuOpening.disconnect(callback);
    }

    function test_UbuntuWebContextMenu17_insertItemAfter() {
      webView.url = "http://testsuite/tst_UbuntuWebContextMenu.html";
      verify(webView.waitForLoadSucceeded());

      function contextMenuOpeningHandler(params, menu) {
        spy.target = menu;
        spy.signalName = "itemsChanged";

        spy2.signalName = "sectionChanged";

        var item1 = menuItemFactory.createObject(null, {});
        var item2 = menuItemFactory.createObject(null, {});

        spy2.target = item1;
        // Verify that calling insertItemAfter with a non-existant stock action
        // behaves as expected
        compare(menu.insertItemAfter(UbuntuWebContextMenuItem.ActionCopy, item1), -1);
        compare(spy.count, 0);
        compare(menu.items.length, 5);
        // The specified item shouldn't be reparented
        compare(TestSupport.qObjectParent(item1), null);
        compare(spy2.count, 0);

        compare(menu.insertItemAfter(UbuntuWebContextMenuItem.ActionSaveLink, item1), 2);
        compare(spy.count, 1);
        compare(menu.items.length, 6);
        // The item should be reparented to the menu
        compare(TestSupport.qObjectParent(item1), menu);
        // Verify that items not inserted in to a section don't have their
        // section property updated
        compare(item1.section, UbuntuWebContextMenuItem.NoSection);
        compare(spy2.count, 0);

        spy2.target = item2;
        spy2.clear();
        compare(menu.insertItemAfter(UbuntuWebContextMenuItem.ActionCopyMediaLocation, item2), 4);
        compare(spy.count, 2);
        compare(menu.items.length, 7);
        compare(TestSupport.qObjectParent(item2), menu);
        // Item's inserted in to a section should have their section property
        // updated accordingly
        compare(item2.section, UbuntuWebContextMenuItem.SectionMedia);
        compare(spy2.count, 1);

        compare(menu.items[2], item1);
        compare(menu.items[4], item2);
      }

      var callback = wrapCallbackTestSequence(contextMenuOpeningHandler);
      webView.contextMenuOpening.connect(callback);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#imagelink");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(callback.wait());

      webView.contextMenuOpening.disconnect(callback);
    }

    function test_UbuntuWebContextMenu18_insertActionAfter() {
      webView.url = "http://testsuite/tst_UbuntuWebContextMenu.html";
      verify(webView.waitForLoadSucceeded());

      function contextMenuOpeningHandler(params, menu) {
        spy.target = menu;
        spy.signalName = "itemsChanged";

        var action3 = actionFactory.createObject(null, {});
        var helper3 = TestSupport.createQObjectTestHelper(action3);

        // Verify that calling insertActionAfter with a non-existant stock
        // action behaves as expected
        compare(menu.insertActionAfter(UbuntuWebContextMenuItem.ActionEraseAll, action3), -1);
        compare(spy.count, 0);
        compare(menu.items.length, 5);
        // The action shouldn't have been reparented or destroyed
        compare(TestSupport.qObjectParent(action3), null);
        verify(!helper3.destroyed);

        compare(menu.insertActionAfter(UbuntuWebContextMenuItem.ActionSaveLink, action3), 2);
        compare(spy.count, 1);
        compare(menu.items.length, 6);
        // The created item should be owned by the menu
        compare(TestSupport.qObjectParent(menu.items[2]), menu);
        compare(menu.items[2].action, action3);
        // The action should have been reparented to the created item
        compare(TestSupport.qObjectParent(action3), menu.items[2]);
        compare(menu.items[2].stockAction, UbuntuWebContextMenuItem.NoAction);
        // Items not inserted in to the middle of a section shouldn't have their
        // section property updated
        compare(menu.items[2].section, UbuntuWebContextMenuItem.NoSection);

        compare(menu.insertActionAfter(UbuntuWebContextMenuItem.ActionCopyMediaLocation, action1), 4);
        compare(spy.count, 2);
        compare(menu.items.length, 7);
        compare(TestSupport.qObjectParent(menu.items[4]), menu);
        compare(menu.items[4].action, action1);
        // Application owned actions shouldn't be reparented
        compare(TestSupport.qObjectParent(action1), actionList);
        compare(menu.items[4].stockAction, UbuntuWebContextMenuItem.NoAction);
        // Verify that the section property is updated accordingly for items
        // inserted in to the middle of a section
        compare(menu.items[4].section, UbuntuWebContextMenuItem.SectionMedia);

        compare(menu.items[2].action, action3);
        compare(menu.items[4].action, action1);
      }

      var callback = wrapCallbackTestSequence(contextMenuOpeningHandler);
      webView.contextMenuOpening.connect(callback);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#imagelink");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(callback.wait());

      webView.contextMenuOpening.disconnect(callback);
    }

    function test_UbuntuWebContextMenu19_removeItem() {
      var menu = menuFactory.createObject(null, {});

      var item1 = menuItemFactory.createObject(null, {});
      var item2 = menuItemFactory.createObject(null, {});
      var item3 = menuItemFactory.createObject(null, {});
      var item4 = menuItemFactory.createObject(null, {});

      menu.appendItem(item1);
      menu.appendItem(item2);
      menu.appendItem(item3);

      compare(menu.items.length, 3);

      spy.target = menu;
      spy.signalName = "itemsChanged";

      compare(TestSupport.qObjectParent(item2), menu);

      // Verify that removing an item correctly unparents it and returns
      // ownership to the caller
      menu.removeItem(item2);
      compare(spy.count, 1);
      compare(menu.items.length, 2);
      compare(TestSupport.qObjectParent(item2), null);

      // Verify that calling removeItem with a null item behaves as expected
      menu.removeItem(null);
      compare(spy.count, 1);
      compare(menu.items.length, 2);

      // Verify that calling removeItem with an item that doesn't belong to
      // this menu works as expected
      menu.removeItem(item4);
      compare(spy.count, 1);
      compare(menu.items.length, 2);

      compare(menu.items[0], item1);
      compare(menu.items[1], item3);
    }

    function test_UbuntuWebContextMenu20_removeItemAt() {
      var menu = menuFactory.createObject(null, {});

      var item1 = menuItemFactory.createObject(null, {});
      var item2 = menuItemFactory.createObject(null, {});
      var item3 = menuItemFactory.createObject(null, {});

      menu.appendItem(item1);
      menu.appendItem(item2);
      menu.appendItem(item3);

      compare(menu.items.length, 3);

      spy.target = menu;
      spy.signalName = "itemsChanged";

      compare(TestSupport.qObjectParent(item2), menu);

      var helper = TestSupport.createQObjectTestHelper(item2);
      menu.removeItemAt(1);
      compare(spy.count, 1);
      compare(menu.items.length, 2);
      // Calling removeItemAt should destroy the removed item
      verify(helper.destroyed);

      // Verify that calling removeItemAt with bogus indices behaves as
      // expected
      menu.removeItemAt(-10);
      compare(spy.count, 1);
      compare(menu.items.length, 2);
      menu.removeItemAt(10);
      compare(spy.count, 1);
      compare(menu.items.length, 2);

      compare(menu.items[0], item1);
      compare(menu.items[1], item3);
    }

    function test_UbuntuWebContextMenu21_removeAll() {
      var menu = menuFactory.createObject(null, {});

      var item1 = menuItemFactory.createObject(null, {});
      var item2 = menuItemFactory.createObject(null, {});
      var item3 = menuItemFactory.createObject(null, {});

      menu.appendItem(item1);
      menu.appendItem(item2);
      menu.appendItem(item3);

      compare(menu.items.length, 3);

      spy.target = menu;
      spy.signalName = "itemsChanged";
      var helper1 = TestSupport.createQObjectTestHelper(item1);
      var helper2 = TestSupport.createQObjectTestHelper(item2);
      var helper3 = TestSupport.createQObjectTestHelper(item3);

      menu.removeAll();
      compare(spy.count, 3);
      compare(menu.items.length, 0);
      // removeAll should destroy all items
      verify(helper1.destroyed);
      verify(helper2.destroyed);
      verify(helper3.destroyed);
    }

    function test_UbuntuWebContextMenu22_deletion() {
      var menu = menuFactory.createObject(null, {});

      var action3 = actionFactory.createObject(null, {});

      menu.appendAction(action1);
      menu.appendAction(action3);

      var helper1 = TestSupport.createQObjectTestHelper(menu.items[0]);
      var helper2 = TestSupport.createQObjectTestHelper(menu.items[1]);
      var helper3 = TestSupport.createQObjectTestHelper(action1);
      var helper4 = TestSupport.createQObjectTestHelper(action3);

      TestSupport.destroyQObjectNow(menu);

      verify(helper1.destroyed);
      verify(helper2.destroyed);
      verify(!helper3.destroyed);
      verify(helper4.destroyed);
    }
  }
}
