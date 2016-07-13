import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  SignalSpy {
    id: spy
  }

  Component.onCompleted: spy.target = context.cookieManager

  TestCase {
    id: test
    name: "CookieManager"
    when: windowShown

    property var last_id: -1

    function init() {
      webView.context.deleteAllCookies();
      spy.clear();
      spy.signalName = "";
    }

    function _verify_id(id) {
      verify(id > last_id);
      last_id = id;
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
      spy.signalName = "getCookiesResponse";
      var id = cookieManager.getAllCookies();
      _verify_id(id);
      spy.wait();
      compare(spy.count, 1);
      compare(spy.signalArguments[0][0], id);

      var numberOfCookiesAtStart = spy.signalArguments[0][1].length;

      // First we set some cookies - we set 1 httponly, 2 with different paths,
      // and 1 session cookie

      spy.signalName = "setCookiesResponse";

      var exp = new Date(Date.now() + 1000*1000);
      id = cookieManager.setCookies(
          "http://testsuite", [
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
      compare(spy.signalArguments[0][1].length, 0);

      // Then we verify the cookies we get back from getCookies() with a
      // specific URL, which should omit one of the cookies we set

      spy.signalName = "getCookiesResponse";

      id = cookieManager.getCookies("http://testsuite/empty.html");
      _verify_id(id);

      spy.wait();
      compare(spy.count, 3, "Expected gotCookies signal");
      compare(spy.signalArguments[0][0], id);

      var cookies = spy.signalArguments[0][1];
      compare(cookies.length, 3);
      _verify_cookies(cookies, [
        {"name": "foo", "value": "bar", "httponly": false, "expirationdate": exp, "domain": "testsuite", "path": "/empty.html", "issecure": false},
        {"name": "blabla", "value": "ddu", "httponly": true, "expirationdate": exp, "domain": "testsuite", "path": "/", "issecure": false},
        {"name": "foofoo", "value": "bla", "httponly": false, "expirationdate": undefined, "domain": "testsuite", "path": "/", "issecure": false}]);

      // Now we verify document.cookie to ensure that the httponly cookie is
      // omitted

      webView.url = "http://testsuite/empty.html";
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
        {"url": "http://www.google.com/", "cookie": {"name": "foo", "value": "bar", "domain": ".mail.google.com"}}, // Domain mismatch
        {"url": "http://testsuite/", "cookie": {"name": "", "value": "foo"}} // No name
      ];
    }

    function test_CookieManager2_errors(data) {
      spy.signalName = "setCookiesResponse";

      var id = webView.context.cookieManager.setCookies(
          data.url, [data.cookie]);
      _verify_id(id);

      spy.wait();
      compare(spy.signalArguments[0][0], id);
      compare(spy.signalArguments[0][1].length, 1);
      compare(spy.signalArguments[0][1][0].name, data.cookie.name);
      compare(spy.signalArguments[0][1][0].value, data.cookie.value);
    }

    function test_CookieManager3_domain_cookie() {
      spy.signalName = "setCookiesResponse";

      var id = webView.context.cookieManager.setCookies(
          "http://foo.testsuite.com/",
          [{"name": "foo", "value": "bar", "domain": ".testsuite.com"}]);
      _verify_id(id);

      spy.wait();
      compare(spy.signalArguments[0][0], id);
      compare(spy.signalArguments[0][1].length, 0);

      webView.url = "http://foo.testsuite.com/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().evaluateCode("return document.cookie;", true), "foo=bar");
    }
  }
}
