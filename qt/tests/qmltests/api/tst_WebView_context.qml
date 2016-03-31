import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestCase {
  id: test
  name: "WebView_context"

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

  // Ensure that WebView.context does not return anything when it is using
  // the default WebContext
  function test_WebView_context1_default() {
    var v = webViewFactory.createObject(null, {});
    compare(v.context, Oxide.defaultWebContext(), "WebView should have a default context"); 
  }

  // Ensure that WebView.context cannot be changed after construction
  function test_WebView_context2_construct_only() {
    var c = webContextFactory.createObject(null, {});
    verify(c);
    var v = webViewFactory.createObject(null, {});
    v.context = c;
    verify(v.context != c, "Shouldn't be able to change the context after construction");
  }

  // Ensure we can set a valid context
  function test_WebView_context4_set_valid() {
    var c = webContextFactory.createObject(null, {});
    verify(c);
    var v = webViewFactory.createObject(null, { context: c });
    compare(v.context, c);

    v.context = null;
    compare(v.context, c);
  }

  // Ensure that WebView.context is cleared if the context is deleted. Note
  // that the underlying BrowserContext remains valid and the WebView will
  // continue to use it
  function test_WebView_context5_delete() {
    var c = webContextFactory.createObject(null, {});
    verify(c);
    var v = webViewFactory.createObject(null, { context: c });
    compare(v.context, c);

    spy.target = v;
    spy.signalName = "contextChanged";

    TestSupport.destroyQObjectNow(c);
    verify(!v.context);
    compare(spy.count, 1);
  }
}
