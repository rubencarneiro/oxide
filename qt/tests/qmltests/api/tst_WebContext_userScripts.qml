import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView.context
    signalName: "userScriptsChanged"
  }

  Component {
    id: userScript
    UserScript {}
  }

  Component {
    id: webContext
    WebContext {}
  }

  TestCase {
    id: test
    name: "WebContext_userScripts"
    when: windowShown

    function init() {
      spy.clear();
    }

    function test_WebContext_userScripts1_add_unowned() {
      var script = userScript.createObject(null, {});
      webView.context.addUserScript(script);

      compare(spy.count, 1, "Should have had a signal");
      compare(webView.context.userScripts.length, 2, "Unexpected number of user scripts");
      compare(webView.context.userScripts[1], script, "Unexpected script");
      compare(OxideTestingUtils.qObjectParent(script), webView.context,
              "UserScript should be owned by the WebContext");

      webView.context.removeUserScript(script);

      compare(spy.count, 2, "Should have had a signal");
      compare(webView.context.userScripts.length, 1, "Unexpected number of user scripts");
      compare(OxideTestingUtils.qObjectParent(script), null,
              "UserScript should have no owner now");
    }

    function test_WebContext_userScripts2_add_already_owned() {
      var otherContext = webContext.createObject(null, {});
      var script = userScript.createObject(null, {});
      otherContext.addUserScript(script);
      compare(OxideTestingUtils.qObjectParent(script), otherContext,
              "UserScript should be owned by the other context");

      webView.context.addUserScript(script);

      compare(spy.count, 1, "Should have had a signal");
      compare(webView.context.userScripts.length, 2, "Unexpected number of user scripts");
      compare(webView.context.userScripts[1], script, "Unexpected script");
      compare(OxideTestingUtils.qObjectParent(script), otherContext,
              "UserScript should still be owned by the other context");

      webView.context.removeUserScript(script);

      compare(spy.count, 2, "Should have had a signal");
      compare(webView.context.userScripts.length, 1, "Unexpected number of user scripts");
      compare(OxideTestingUtils.qObjectParent(script), otherContext,
              "UserScript should still be owned by the other context");
    }

    function test_WebContext_userScripts3_deleted_by_someone_else() {
      var otherContext = webContext.createObject(null, {});
      var script = userScript.createObject(null, {});
      otherContext.addUserScript(script);
      compare(OxideTestingUtils.qObjectParent(script), otherContext,
              "UserScript should be owned by the other context");

      webView.context.addUserScript(script);

      compare(spy.count, 1, "Should have had a signal");
      compare(webView.context.userScripts.length, 2, "Unexpected number of user scripts");
      compare(webView.context.userScripts[1], script, "Unexpected script");
      compare(OxideTestingUtils.qObjectParent(script), otherContext,
              "UserScript should still be owned by the other context");

      var obs = OxideTestingUtils.createDestructionObserver(script);
      otherContext.destroy();
      verify(webView.waitFor(function() { return obs.destroyed; }));

      compare(spy.count, 2, "Should have had a signal");
      compare(webView.context.userScripts.length, 1, "Unexpected number of user scripts");
    }

    function test_WebContext_userScripts4_readd_unowned() {
      var otherContext = webContext.createObject(null, {});
      var script = userScript.createObject(null, {});
      otherContext.addUserScript(script);

      webView.context.addUserScript(script);

      otherContext.removeUserScript(script);
      compare(OxideTestingUtils.qObjectParent(script), null,
              "UserScript should be unowned");

      webView.context.addUserScript(script);
      compare(OxideTestingUtils.qObjectParent(script), webView.context,
              "Context should now own UserScript");

      webView.context.removeUserScript(script);
    }
  }
}
