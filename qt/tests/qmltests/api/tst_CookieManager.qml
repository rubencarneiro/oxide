import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView

  SignalSpy {
    id: spy
    target: context.cookieManager
  }

  TestCase {
    id: test
    name: "CookieManager"
    when: windowShown

    property var expected_id: -1

    function init() {
      webView.url = "http://localhost:8080/clear-test-cookies-hack.py"
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      spy.clear();
      spy.signalName = "";
    }

    function _verify_id(id) {
      if (expected_id == -1) {
        verify(id > -1, "Invalid ID");
      } else {
        compare(id, expected_id, "Invalid ID");
      }
      expected_id = id + 1; 
    }

    function _verify_cookies(cookies, data) {
      for (var i = 0; i < data.length; ++i) {
        compare(cookies[i].name, data[i].name);
        compare(cookies[i].value, data[i].value);
        compare(cookies[i].domain, data[i].domain);
        compare(cookies[i].path, data[i].path);
        compare(cookies[i].httponly, data[i].httponly);
        compare(cookies[i].issecure, data[i].issecure);
        compare(cookies[i].expirationdate, data[i].expirationdate);
        if (cookies[i].expirationdate === undefined) {
          verify(cookies[i].issessioncookie);
        } else {
          verify(!cookies[i].issessioncookie);
        }
      }
    }

    function test_WebContext_cookieManager1() {
      var cookieManager = webView.context.cookieManager;
      verify(cookieManager, "CookieManager is NULL");

      // Save the total number of cookies at the start
      spy.signalName = "gotCookies";
      var id = cookieManager.getAllCookies();
      _verify_id(id);
      spy.wait();
      compare(spy.count, 1);
      compare(spy.signalArguments[0][0], id);

      var numberOfCookiesAtStart = spy.signalArguments[0][1].length;

      // First we set some cookies - we set 1 httponly, 2 with different paths,
      // and 1 session cookie

      spy.signalName = "cookiesSet";

      var exp = new Date(Date.now() + 1000*1000);
      id = cookieManager.setCookies(
          "http://localhost:8080", [
          {"name": "blabla",
           "value": "ddu",
           "httponly": true,
	       "expirationdate": exp},
          {"name": "foo",
           "value": "bar",
           "path": "/empty.html",
           "expirationdate": exp},
          {"name": "test",
           "value": "bleurgh",
           "path": "/moo.html"},
          {"name": "foofoo",
           "value": "bla"}]);
      _verify_id(id);

      spy.wait();
      compare(spy.count, 2, "Expected cookiesSet signal");
      compare(spy.signalArguments[0][0], id);
      compare(spy.signalArguments[0][1], CookieManager.RequestStatusOK);

      // Then we verify the cookies we get back from getCookies() with a
      // specific URL, which should omit one of the cookies we set

      spy.signalName = "gotCookies";

      id = cookieManager.getCookies("http://localhost:8080/empty.html");
      _verify_id(id);

      spy.wait();
      compare(spy.count, 3, "Expected gotCookies signal");
      compare(spy.signalArguments[0][0], id);

      var cookies = spy.signalArguments[0][1];
      compare(cookies.length, 3);
      _verify_cookies(cookies, [
        {"name": "foo", "value": "bar", "httponly": false, "expirationdate": exp, "domain": "localhost", "path": "/empty.html", "issecure": false},
        {"name": "blabla", "value": "ddu", "httponly": true, "expirationdate": exp, "domain": "localhost", "path": "/", "issecure": false},
        {"name": "foofoo", "value": "bla", "httponly": false, "expirationdate": undefined, "domain": "localhost", "path": "/", "issecure": false}]);

      // Now we verify document.cookie to ensure that the httponly cookie is
      // omitted

      webView.url = "http://localhost:8080/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().evaluateCode("return document.cookie;", true),
              "foo=bar; foofoo=bla");

      // Now verify that getAllCookies() returns the expected number of cookies

      id = cookieManager.getAllCookies();
      _verify_id(id);

      spy.wait();
      compare(spy.count, 4);
      compare(spy.signalArguments[1][0], id);

      compare(spy.signalArguments[1][1].length, numberOfCookiesAtStart + 4);
    }

    function test_CookieManager2_errors_data() {
      return [
        {"url": "http://www.google.com/", "cookie": {"name": "foo", "value": "bar", "domain": ".mail.google.com"}}
      ];
    }

    function test_CookieManager2_errors(data) {
      spy.signalName = "cookiesSet";

      var id = webView.context.cookieManager.setCookies(
          data.url, [data.cookie]);
      _verify_id(id);

      spy.wait();
      compare(spy.signalArguments[0][0], id);
      compare(spy.signalArguments[0][1], CookieManager.RequestStatusError);
    }
  }
}
