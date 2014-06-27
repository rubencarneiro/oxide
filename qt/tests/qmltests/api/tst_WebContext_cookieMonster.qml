import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView

  SignalSpy {
    id: cookiesSetSpy
    target: webView.context.cookieMonster
    signalName: "cookiesSet"
  }
  SignalSpy {
    id: gotCookiesSpy
    target: webView.context.cookieMonster
    signalName: "gotCookies"
  }

  TestCase {
    id: test
    name: "WebContext_cookieMonster"
    when: windowShown

    function init() {
      webView.context.cookiePolicy = WebContext.CookiePolicyAllowAll;
      _clear();
    }

    function _clear() {
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

    function test_WebContext_cookieMonster_getAll() {
      _set_cookies(webView);

      var cookieMonster = webView.context.cookieMonster;
      verify(cookieMonster, "CookieMonster is NULL");

      cookieMonster.getAllCookies();
      gotCookiesSpy.wait();
      compare(gotCookiesSpy.count, 1, "Expected gotCookies signal");
      var cookieList = gotCookiesSpy.signalArguments[0];
      compare(cookieList.length, 1);
    }
  }
}
