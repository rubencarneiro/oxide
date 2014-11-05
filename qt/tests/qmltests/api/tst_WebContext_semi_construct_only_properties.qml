import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestCase {
  id: test
  name: "WebContext_semi_construct_only_properties"

  Component {
    id: webViewFactory
    WebView {}
  }

  Component {
    id: webContextFactory
    WebContext {}
  }

  SignalSpy {
    id: spy
  }

  function init() {
    spy.clear();
  }

  function test_WebContext_semi_construct_only_properties1_data() {
    var r = [
      { prop: "dataPath", signal: "dataPathChanged", val: "file:///foo", dataPath: "" },
      { prop: "cachePath", signal: "cachePathChanged", val: "file:///foo", dataPath: "" }
    ];
    if (QMLTEST_USE_CONTEXT_DATADIR) {
      r.push(
        { prop: "sessionCookieMode", signal: "sessionCookieModeChanged", val: WebContext.SessionCookieModeRestored, dataPath: QMLTEST_DATADIR }
      );
    }

    return r;
  }

  // This verifies that the various semi-constuct only properties
  // (those that can be modified until the WebContext is in use)
  // work correctly
  function test_WebContext_semi_construct_only_properties1(data) {
    var c = webContextFactory.createObject(null, { dataPath: data.dataPath });
    spy.target = c;
    spy.signalName = data.signal;

    var old = c[data.prop];
    verify(old != data.val);

    c[data.prop] = data.val;
    compare(spy.count, 1, "Should have had a signal");
    compare(c[data.prop], data.val, "Unexpected value read back");

    var v = webViewFactory.createObject(null, { context: c });

    c[data.prop] = old;
    compare(spy.count, 1, "Shouldn't have had a signal");
    compare(c[data.prop], data.val, "Should have read old value back");

    OxideTestingUtils.destroyQObjectNow(v);
    OxideTestingUtils.destroyQObjectNow(c);
  }
}
