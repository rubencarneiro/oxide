import QtQuick 2.0
import QtTest 1.0
import Oxide.testsupport 1.0
import Oxide.Ubuntu 1.0
import Ubuntu.Components 1.3

TestCase {
  id: test
  name: "UbuntuWebContextMenuItem"

  Component {
    id: menuItemFactory
    UbuntuWebContextMenuItem {}
  }

  Component {
    id: menuFactory
    UbuntuWebContextMenu {}
  }

  Action {
    id: action
  }

  Component {
    id: actionFactory
    Action {}
  }

  SignalSpy {
    id: spy
  }

  function init() {
    spy.target = null;
    spy.signalName = "";
    spy.clear();
  }

  function test_UbuntuWebContextMenuItem1_default() {
    var item = menuItemFactory.createObject(null, {});
    compare(item.stockAction, UbuntuWebContextMenuItem.NoAction);
    compare(item.section, UbuntuWebContextMenuItem.NoSection);
    verify(!item.action);
  }

  function test_UbuntuWebContextMenuItem2_action_signal() {
    var item = menuItemFactory.createObject(null, {});
    spy.target = item;
    spy.signalName = "actionChanged";

    item.action = action;
    compare(spy.count, 1);

    item.action = action;
    compare(spy.count, 1);

    item.action = null;
    compare(spy.count, 2);
  }

  function test_UbuntuWebContextMenuItem3_action_app_owned() {
    var item = menuItemFactory.createObject(null, {});

    item.action = action;
    compare(TestSupport.qObjectParent(action), test);

    var helper = TestSupport.createQObjectTestHelper(action);

    item.action = null;
    verify(!helper.destroyed);

    item.action = action;
    TestSupport.destroyQObjectNow(item);
    verify(!helper.destroyed);
  }

  function test_UbuntuWebContextMenuItem4_action_item_owned() {
    var item = menuItemFactory.createObject(null, {});
    var action = actionFactory.createObject(null, {});

    item.action = action;
    compare(TestSupport.qObjectParent(action), item);

    var helper = TestSupport.createQObjectTestHelper(action);

    item.action = null;
    verify(helper.destroyed);

    action = actionFactory.createObject(null, {});
    helper = TestSupport.createQObjectTestHelper(action);

    item.action = action;
    TestSupport.destroyQObjectNow(item);
    verify(helper.destroyed);
  }

  function test_UbuntuWebContextMenuItem5_destruction_removes_from_menu() {
    var menu = menuFactory.createObject(null, {});
    var item = menuItemFactory.createObject(null, {});

    menu.appendItem(item);
    compare(menu.items.length, 1);
    compare(menu.items[0], item);

    spy.target = menu;
    spy.signalName = "itemsChanged";

    TestSupport.destroyQObjectNow(item);

    compare(spy.count, 1);
    compare(menu.items.length, 0);
  }
}
