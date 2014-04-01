import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestCase {
  id: test
  name: "WebContext_sessionCookies"

  function _createWebView(sessionCookieMode) {
    var webContext = webContextComponent.createObject(this, {
      "sessionCookieMode": sessionCookieMode
    });
    var webView = webViewComponent.createObject(this, {
      "context": webContext
    });
    return webView;
  }

  function _deleteWebView(webView) {
    var context = webView.context;
    webView.destroy();
    context.destroy();
  }

  function _clear(webView) {
    var restore = webView.context.cookiePolicy;
    webView.context.cookiePolicy = WebContext.CookiePolicyAllowAll;

    webView.url = "http://localhost:8080/clear-test-cookies-hack.py"
    verify(webView.waitForLoadSucceeded(),
           "Timed out waiting for successful load");

    webView.context.cookiePolicy = restore;
  }

  function _set_cookies(webView) {
    webView.url = "http://localhost:8080/tst_WebContext_sessionCookies.py";
    verify(webView.waitForLoadSucceeded(),
           "Timed out waiting for successful load");
  }

  function _test_can_get_session_cookies(webView) {
    webView.url = "http://localhost:8080/get-cookies.py";
    verify(webView.waitForLoadSucceeded(),
           "Timed out waiting for successful load");

    var cookies = JSON.parse(webView.getTestApi().evaluateCode(
        "return document.body.children[0].innerHTML", true));

    return "sessionfoo" in cookies && cookies["sessionfoo"] == "sessionbar";
  }

  function test_WebContext_sessionCookies_ephemeral() {
    if (OxideTestingUtils.DATA_PATH == "") {
      skip("Can't run session cookie tests withour a permanent storage");
    }

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
    if (OxideTestingUtils.DATA_PATH == "") {
      skip("Can't run session cookie tests withour a permanent storage");
    }

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
    if (OxideTestingUtils.DATA_PATH == "") {
      skip("Can't run session cookie tests withour a permanent storage");
    }
      
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
