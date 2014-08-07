import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView

  property var latestCookieList

  SignalSpy {
    id: cookiesSetSpy
    signalName: "cookiesSet"
  }
  SignalSpy {
    id: gotCookiesSpy
    signalName: "gotCookies"
  }

  TestCase {
    id: test
    name: "WebContext_cookieManager"
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
      latestCookieList = null
    }

    function _set_cookies(webView) {
      webView.url = "http://localhost:8080/tst_WebContext_sessionCookies.py";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }

    function test_WebContext_cookieManager_setCookies() {
      var cookieManager = webView.context.cookieManager;
      verify(cookieManager, "CookieManager is NULL");

      cookiesSetSpy.target = cookieManager;
      gotCookiesSpy.target = cookieManager;

      var set_id = cookieManager.setCookies("http://",
        [{"name": "blabla",
	  "value": "ddu",
	  "expirationdate": Date.now() + 1000*1000}]);

      verify(set_id >= 0, "Invalid cookie request id");
      cookiesSetSpy.wait();
      compare(cookiesSetSpy.count, 1, "Expected cookiesSet signal");
      compare(cookiesSetSpy.signalArguments[0][0], set_id)
      compare(cookiesSetSpy.signalArguments[0][1], true)

      var get_id = cookieManager.getAllCookies();
      verify(get_id >= 0, "Invalid cookie request id");
      gotCookiesSpy.wait();
      compare(gotCookiesSpy.count, 1, "Expected gotCookies signal");
    }

    function test_WebContext_cookieManager_getAllCookies() {
      _set_cookies(webView);

      var cookieManager = webView.context.cookieManager;
      verify(cookieManager, "CookieManager is NULL");

      cookiesSetSpy.target = cookieManager;
      gotCookiesSpy.target = cookieManager;
      cookieManager.gotCookies.connect(_on_got_cookies);
      var id = cookieManager.getAllCookies();
      verify(id >= 0, "Invalid cookie request id");
      gotCookiesSpy.wait();

      compare(gotCookiesSpy.count, 1, "Expected gotCookies signal");
      compare(latestCookieList.length, 1);
    }

    function _on_got_cookies(requestId, networkCookies) {
      console.log("Parsed: " + JSON.stringify(networkCookies.rawHttpCookies))
      latestCookieList = [].concat(networkCookies.rawHttpCookies)
    }
  }
}
