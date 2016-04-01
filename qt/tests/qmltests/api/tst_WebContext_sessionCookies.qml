import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestCase {
  id: test
  name: "WebContext_sessionCookies"

  function _createWebView(sessionCookieMode) {
    var webContext = webContextComponent.createObject(this, {
      "dataPath": QMLTEST_TMPDIR + "/_test_context",
      "sessionCookieMode": sessionCookieMode
    });
    var webView = webViewComponent.createObject(this, {
      "context": webContext
    });
    return webView;
  }

  function _deleteWebView(webView) {
    var context = webView.context;
    TestSupport.destroyQObjectNow(webView);
    TestSupport.destroyQObjectNow(context);
    // XXX: Hack to ensure that the cookie store lock gets released before
    // starting the next test
    TestSupport.wait(200);
  }

  function _clear(webView) {
    webView.context.deleteAllCookies();
  }

  function _set_cookies(webView) {
    webView.url = "http://testsuite/tst_WebContext_sessionCookies.py";
    verify(webView.waitForLoadSucceeded(),
           "Timed out waiting for successful load");
  }

  function _test_can_get_session_cookies(webView) {
    webView.url = "http://testsuite/get-cookies.py";
    verify(webView.waitForLoadSucceeded(),
           "Timed out waiting for successful load");

    var cookies = JSON.parse(webView.getTestApi().evaluateCode(
        "return document.body.children[0].innerHTML", true));

    return "sessionfoo" in cookies && cookies["sessionfoo"] == "sessionbar";
  }

  function test_WebContext_sessionCookies_ephemeral() {
    var webView = _createWebView(WebContext.SessionCookieModeEphemeral);
    _clear(webView);
    _set_cookies(webView);
    _deleteWebView(webView);

    webView = _createWebView(WebContext.SessionCookieModeRestored);
    verify(!_test_can_get_session_cookies(webView),
           "Shouldn't be able to read session cookies");
    _deleteWebView(webView);
  }

  function test_WebContext_sessionCookies_persistent() {
    var webView = _createWebView(WebContext.SessionCookieModeRestored);
    _clear(webView);
    _set_cookies(webView);
    _deleteWebView(webView);

    // In this mode, we can't load the session cookies but we can write them
    webView = _createWebView(WebContext.SessionCookieModePersistent);
    verify(!_test_can_get_session_cookies(webView),
           "Shouldn't be able to read session cookies");
    _set_cookies(webView);
    _deleteWebView(webView);

    // Session cookies written while in persistent mode should be there
    webView = _createWebView(WebContext.SessionCookieModeRestored);
    verify(_test_can_get_session_cookies(webView),
           "Should be able to read session cookies");
    _deleteWebView(webView);
  }

  function test_WebContext_sessionCookies_restored() {
    var webView = _createWebView(WebContext.SessionCookieModeRestored);
    _clear(webView);
    _set_cookies(webView);
    _deleteWebView(webView);

    webView = _createWebView(WebContext.SessionCookieModeRestored);
    verify(_test_can_get_session_cookies(webView),
           "Should be able to read session cookies");
    _deleteWebView(webView);
  }

  Component {
    id: webViewComponent
    TestWebView {
      focus: true
      width: 200
      height: 200
    }
  }

  Component {
    id: webContextComponent
    TestWebContext {}
  }
}
