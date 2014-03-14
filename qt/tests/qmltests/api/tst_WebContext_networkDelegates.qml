import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: top

  WebContext {
    id: context
  }

  WebContext {
    id: context2
  }

  SignalSpy {
    id: spy
    target: context
  }

  Component {
    id: "networkDelegateWorkerFactory"
    NetworkDelegateWorker {}
  }

  TestCase {
    id: test
    name: "WebContext_networkDelegates"
    when: windowShown

    function init() {
      spy.signalName = "";
      spy.clear();
    }

    property var slots: [
      { prop: "networkRequestDelegate", signal: "networkRequestDelegateChanged" },
      { prop: "storageAccessPermissionDelegate", signal: "storageAccessPermissionDelegateChanged" }
    ]

    function test_WebContext_networkDelegates1_assign_unparented_data() {
      return slots;
    }

    function test_WebContext_networkDelegates1_assign_unparented(data) {
      spy.signalName = data.signal;

      var d = networkDelegateWorkerFactory.createObject(null, {});

      context[data.prop] = d;
      compare(context[data.prop], d, "Unexpected value");
      compare(spy.count, 1, "Expected a signal");
      compare(OxideTestingUtils.qObjectParent(d), context,
              "Delegate should be parented to the WebContext");

      context[data.prop] = d;
      compare(spy.count, 1, "Shouldn't have had another signal");

      context[data.prop] = null;
      compare(spy.count, 2, "Expected a signal");
      compare(OxideTestingUtils.qObjectParent(d), null,
              "Delegate should not have a parent");
    }

    function test_WebContext_networkDelegates2_assign_already_in_use_data() {
      return slots;
    }

    function test_WebContext_networkDelegates2_assign_already_in_use(data) {
      spy.signalName = data.signal;

      var d = networkDelegateWorkerFactory.createObject(null, {});
      context2[data.prop] = d;
      context[data.prop] = d;
      compare(spy.count, 0, "Shouldn't have had a signal");
      compare(context[data.prop], null, "Unexpected value");
      compare(OxideTestingUtils.qObjectParent(d), context2,
              "Shouldn't have been reparented");

      context2[data.prop] = null;
    }

    function test_WebContext_networkDelegates3_reparent_data() {
      return slots;
    }

    function test_WebContext_networkDelegates3_reparent(data) {
      spy.signalName = data.signal;

      var d = networkDelegateWorkerFactory.createObject(top, {});
      context[data.prop] = d;
      compare(context[data.prop], d, "Unexpected value");
      compare(spy.count, 1, "Expected a signal");
      compare(OxideTestingUtils.qObjectParent(d), context,
              "Delegate should be parented to the WebContext");
    }

    function test_WebContext_networkDelegates4_delete_in_use_data() {
      return slots;
    }

    function test_WebContext_networkDelegates4_delete_in_use(data) {
      spy.signalName = data.signal;

      var d = networkDelegateWorkerFactory.createObject(null, {});

      context[data.prop] = d;
      compare(context[data.prop], d, "Unexpected value");
      compare(spy.count, 1, "Expected a signal");
      compare(OxideTestingUtils.qObjectParent(d), context,
              "Delegate should be parented to the WebContext");

      var obs = OxideTestingUtils.createDestructionObserver(d);
      d.destroy();
      verify(top.waitFor(function() { return obs.destroyed; }),
             "Failed to destroy object");
      
      compare(spy.count, 2, "Expected a signal");
      compare(context[data.prop], null, "Value should have been cleared");
    }

    function test_WebContext_networkDelegates5_shared() {
      var d = networkDelegateWorkerFactory.createObject(null, {});
      context.networkRequestDelegate = d;
      context.storageAccessPermissionDelegate = d;

      compare(context.networkRequestDelegate, d);
      compare(context.storageAccessPermissionDelegate, d);
      compare(OxideTestingUtils.qObjectParent(d), context);

      context.networkRequestDelegate = null;

      compare(context.networkRequestDelegate, null);
      compare(context.storageAccessPermissionDelegate, d);
      compare(OxideTestingUtils.qObjectParent(d), context);

      context.storageAccessPermissionDelegate = null;

      compare(context.networkRequestDelegate, null);
      compare(context.storageAccessPermissionDelegate, null);
      compare(OxideTestingUtils.qObjectParent(d), null);
    }
  }
}
