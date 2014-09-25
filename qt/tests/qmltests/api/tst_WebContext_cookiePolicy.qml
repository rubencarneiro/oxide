import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  TestCase {
    id: test
    name: "WebContext_cookiePolicy"
    when: windowShown

    function init() {
      webView.context.cookiePolicy = WebContext.CookiePolicyAllowAll;
      _clear();
    }

    function _clear() {
      webView.context.deleteAllCookies();
    }

    function _test_can_set_first_party() {
      _clear();

      webView.url = "http://testsuite/tst_WebContext_cookiePolicy.py";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var restore = webView.context.cookiePolicy;
      webView.context.cookiePolicy = WebContext.CookiePolicyAllowAll;

      webView.url = "http://testsuite/get-cookies.py";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var cookies = JSON.parse(webView.getTestApi().evaluateCode(
          "return document.body.children[0].innerHTML", true));

      webView.context.cookiePolicy = restore;

      return "foo" in cookies && cookies["foo"] == "bar";
    }

    function _test_can_get_first_party() {
      _clear();

      var restore = webView.context.cookiePolicy;
      webView.context.cookiePolicy = WebContext.CookiePolicyAllowAll;

      webView.url = "http://testsuite/tst_WebContext_cookiePolicy.py";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.context.cookiePolicy = restore;

      webView.url = "http://testsuite/get-cookies.py";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var cookies = JSON.parse(webView.getTestApi().evaluateCode(
          "return document.body.children[0].innerHTML", true));

      return "foo" in cookies && cookies["foo"] == "bar";
    }

    function _test_can_set_third_party() {
      _clear();

      webView.url = Qt.resolvedUrl("tst_WebContext_cookiePolicy_set_third_party.html");
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var restore = webView.context.cookiePolicy;
      webView.context.cookiePolicy = WebContext.CookiePolicyAllowAll;

      webView.url = "http://testsuite/get-cookies.py";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var cookies = JSON.parse(webView.getTestApi().evaluateCode(
          "return document.body.children[0].innerHTML", true));

      webView.context.cookiePolicy = restore;

      return "foo" in cookies && cookies["foo"] == "bar";
    }

    function _test_can_get_third_party() {
      _clear();

      var restore = webView.context.cookiePolicy;
      webView.context.cookiePolicy = WebContext.CookiePolicyAllowAll;

      webView.url = "http://testsuite/tst_WebContext_cookiePolicy.py";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.context.cookiePolicy = restore;

      webView.url = Qt.resolvedUrl("tst_WebContext_cookiePolicy_get_third_party.html");
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var cookies = JSON.parse(webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode(
          "return document.body.children[0].innerHTML", true));

      return "foo" in cookies && cookies["foo"] == "bar";
    }

    function test_WebContext_cookiePolicy1_allowAll() {
      webView.context.cookiePolicy = WebContext.CookiePolicyAllowAll;

      verify(_test_can_set_first_party(),
             "Should be able to set a first-party cookie");
      verify(_test_can_get_first_party(),
             "Should be able to get a first-party cookie");
      verify(_test_can_set_third_party(),
             "Should be able to set a third-party cookie");
      verify(_test_can_get_third_party(),
             "Should be able to get a third-party cookie");
    }

    function test_WebContext_cookiePolicy2_blockAll() {
      webView.context.cookiePolicy = WebContext.CookiePolicyBlockAll;

      verify(!_test_can_set_first_party(),
             "Shouldn't be able to set a first-party cookie");
      verify(!_test_can_get_first_party(),
             "Shouldn't be able to get a first-party cookie");
      verify(!_test_can_set_third_party(),
             "Shouldn't be able to set a third-party cookie");
      verify(!_test_can_get_third_party(),
             "Shouldn't be able to get a third-party cookie");
    }

    function test_WebContext_cookiePolicy3_blockThirdParty() {
      webView.context.cookiePolicy = WebContext.CookiePolicyBlockThirdParty;

      verify(_test_can_set_first_party(),
             "Should be able to set a first-party cookie");
      verify(_test_can_get_first_party(),
             "Should be able to get a first-party cookie");
      verify(!_test_can_set_third_party(),
             "Shouldn't be able to set a third-party cookie");
      verify(!_test_can_get_third_party(),
             "Should be able to get a third-party cookie");
    }
  }
}
