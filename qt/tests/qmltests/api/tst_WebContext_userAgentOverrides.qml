import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView.context
    signalName: "userAgentOverridesChanged"
  }

  TestCase {
    id: test
    name: "WebContext_userAgentOverrides"
    when: windowShown

    property var kOverrideSet: [
      [ /^http:\/\/1\.testsuite\//, "Override 1" ],
      [ /^http:\/\/2\.testsuite\//, "Override 2" ],
      [ /^http:\/\/3\.testsuite\//, "Override 3" ],
      [ /^http:\/\/4\.testsuite\//, "Override 4" ],
      [ /^http:\/\/5\.testsuite\//, "Override 5" ],
      [ /^http:\/\/6\.testsuite\//, "Override 6" ],
      [ /^http:\/\/7\.testsuite\//, "Override 7" ],
      [ /^http:\/\/8\.testsuite\//, "Override 8" ],
      [ /^http:\/\/9\.testsuite\//, "Override 9" ],
      [ /^http:\/\/10\.testsuite\//, "Override 10" ],
      [ /^http:\/\/11\.testsuite\//, "Override 11" ],
      [ /^http:\/\/12\.testsuite\//, "Override 12" ],
      [ /^http:\/\/13\.testsuite\//, "Override 13" ],
      [ /^http:\/\/14\.testsuite\//, "Override 14" ],
      [ /^http:\/\/15\.testsuite\//, "Override 15" ],
    ]

    function init() {
      webView.context.userAgentOverrides = [];
      webView.context.userAgent = "Default user agent";
      spy.clear();
    }

    function test_WebContext_userAgentOverrides1_data_validity_data() {
      return [
        { data: [ /^http:\/\/.*/ ], valid: false },
        { data: [ /^http:\/\/.*/, "Foo", 2 ], valid: false },
        { data: /^http:\/\/.*/, valid: false },
        { data: [ /^http:\/\/.*/, "Foo" ], valid: true, readback: [ "^http:\\/\\/.*", "Foo" ] },
        { data: [ "^http:\\/\\/.*", "Foo" ], valid: true, readback: [ "^http:\\/\\/.*", "Foo" ] },
        { data: [ "^http:\\/\\/[\\w\\d*", "Foo" ], valid: false }
      ];
    }

    function test_WebContext_userAgentOverrides1_data_validity(data) {
      webView.context.userAgentOverrides = [ data.data ];

      compare(spy.count, data.valid ? 1 : 0);
      compare(webView.context.userAgentOverrides.length, data.valid ? 1 : 0);
      if (data.valid) {
        compare(webView.context.userAgentOverrides[0], data.readback);
      }
    }

    function test_WebContext_userAgentOverrides2_navigator_data() {
      return [
        { url : "http://testsuite/get-headers.py", expected: "Default user agent" },
        { url : "http://1.testsuite/get-headers.py", expected: "Override 1" },
        { url : "http://2.testsuite/get-headers.py", expected: "Override 2" },
        { url : "http://3.testsuite/get-headers.py", expected: "Override 3" },
        { url : "http://4.testsuite/get-headers.py", expected: "Override 4" },
        { url : "http://5.testsuite/get-headers.py", expected: "Override 5" },
        { url : "http://6.testsuite/get-headers.py", expected: "Override 6" },
        { url : "http://7.testsuite/get-headers.py", expected: "Override 7" },
        { url : "http://8.testsuite/get-headers.py", expected: "Override 8" },
        { url : "http://9.testsuite/get-headers.py", expected: "Override 9" },
        { url : "http://10.testsuite/get-headers.py", expected: "Override 10" },
        { url : "http://11.testsuite/get-headers.py", expected: "Override 11" },
        { url : "http://12.testsuite/get-headers.py", expected: "Override 12" },
        { url : "http://13.testsuite/get-headers.py", expected: "Override 13" },
        { url : "http://14.testsuite/get-headers.py", expected: "Override 14" },
        { url : "http://15.testsuite/get-headers.py", expected: "Override 15" },
        { url : "http://foo.15.testsuite/get-headers.py", expected: "Default user agent" },
        { url : "http://16.testsuite/get-headers.py", expected: "Default user agent" },
        { url : "http://1.testsuite/tst_WebContext_userAgentOverrides_redirect.py", expected: "Override 2" },
      ];
    }

    function test_WebContext_userAgentOverrides2_navigator(data) {
      webView.context.userAgentOverrides = kOverrideSet;

      webView.url = data.url;
      verify(webView.waitForLoadSucceeded());

      compare(webView.getTestApi().evaluateCode("return navigator.userAgent", true),
              data.expected);
    }

    function test_WebContext_userAgentOverrides3_http_data() {
      return test_WebContext_userAgentOverrides2_navigator_data();
    }

    function test_WebContext_userAgentOverrides3_http(data) {
      webView.context.userAgentOverrides = kOverrideSet;

      webView.url = data.url;
      verify(webView.waitForLoadSucceeded());

      var headers = JSON.parse(
          webView.getTestApi().evaluateCode(
            "return document.body.children[0].innerHTML", true));
      compare(headers["user-agent"], data.expected);
    }

    function test_WebContext_userAgentOverrides4_update(data) {
      webView.context.userAgentOverrides = kOverrideSet;

      webView.url = "http://5.testsuite/get-headers.py";
      verify(webView.waitForLoadSucceeded());

      var headers = JSON.parse(
          webView.getTestApi().evaluateCode(
            "return document.body.children[0].innerHTML", true));
      compare(headers["user-agent"], "Override 5");

      compare(webView.getTestApi().evaluateCode("return navigator.userAgent", true),
              "Override 5");

      webView.context.userAgentOverrides = [
        [ /^http:\/\/[^\.]\.testsuite\//, "Updated override" ]
      ];

      compare(webView.getTestApi().evaluateCode("return navigator.userAgent", true),
              "Updated override");

      webView.reload();
      verify(webView.waitForLoadSucceeded());

      var headers = JSON.parse(
          webView.getTestApi().evaluateCode(
            "return document.body.children[0].innerHTML", true));
      compare(headers["user-agent"], "Updated override");

      compare(webView.getTestApi().evaluateCode("return navigator.userAgent", true),
              "Updated override");

      webView.context.userAgentOverrides = [];

      compare(webView.getTestApi().evaluateCode("return navigator.userAgent", true),
              "Default user agent");

      webView.reload();
      verify(webView.waitForLoadSucceeded());

      var headers = JSON.parse(
          webView.getTestApi().evaluateCode(
            "return document.body.children[0].innerHTML", true));
      compare(headers["user-agent"], "Default user agent");

      compare(webView.getTestApi().evaluateCode("return navigator.userAgent", true),
              "Default user agent");
    }
  }
}
