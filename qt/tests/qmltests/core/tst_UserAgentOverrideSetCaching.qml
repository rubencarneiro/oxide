import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.9
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  context.userAgent: "Default user agent"

  property variant qtest_overrides: [
    [ /^http:\/\/testsuite\/empty\.html\?1$/, "Override 1" ],
    [ /^http:\/\/testsuite\/empty\.html\?2$/, "Override 2" ],
    [ /^http:\/\/testsuite\/empty\.html\?3$/, "Override 3" ],
    [ /^http:\/\/testsuite\/empty\.html\?4$/, "Override 4" ],
    [ /^http:\/\/testsuite\/empty\.html\?5$/, "Override 5" ],
    [ /^http:\/\/testsuite\/empty\.html\?6$/, "Override 6" ],
    [ /^http:\/\/testsuite\/empty\.html\?7$/, "Override 7" ],
    [ /^http:\/\/testsuite\/empty\.html\?8$/, "Override 8" ],
    [ /^http:\/\/testsuite\/empty\.html\?9$/, "Override 9" ],
    [ /^http:\/\/testsuite\/empty\.html\?10$/, "Override 10" ],
    [ /^http:\/\/testsuite\/empty\.html\?11$/, "Override 11" ],
    [ /^http:\/\/testsuite\/empty\.html\?12$/, "Override 12" ],
    [ /^http:\/\/testsuite\/empty\.html\?13$/, "Override 13" ],
    [ /^http:\/\/testsuite\/empty\.html\?14$/, "Override 14" ],
    [ /^http:\/\/testsuite\/empty\.html\?15$/, "Override 15" ],
  ]

  property variant qtest_data: [
    { url: "http://testsuite/empty.html?1", expected: "Override 1" },
    { url: "http://testsuite/empty.html?2", expected: "Override 2" },
    { url: "http://testsuite/empty.html?3", expected: "Override 3" },
    { url: "http://testsuite/empty.html?4", expected: "Override 4" },
    { url: "http://testsuite/empty.html?5", expected: "Override 5" },
    { url: "http://testsuite/empty.html?6", expected: "Override 6" },
    { url: "http://testsuite/empty.html?7", expected: "Override 7" },
    { url: "http://testsuite/empty.html?8", expected: "Override 8" },
    { url: "http://testsuite/empty.html?9", expected: "Override 9" },
    { url: "http://testsuite/empty.html?10", expected: "Override 10" },
    { url: "http://testsuite/empty.html?11", expected: "Override 11" },
    { url: "http://testsuite/empty.html?12", expected: "Override 12" },
    { url: "http://testsuite/empty.html?13", expected: "Override 13" },
    { url: "http://testsuite/empty.html?14", expected: "Override 14" },
    { url: "http://testsuite/empty.html?15", expected: "Override 15" },
    { url: "http://testsuite/empty.html", expected: "Default user agent" },
  ]

  Component.onCompleted: {
    context.userAgentOverrides = qtest_overrides;
  }

  TestCase {
    id: test
    name: "UserAgentOverrideSetCaching"
    when: windowShown

    function test_UserAgentOverrideSetCaching_data() {
      var data = [];
      for (var i = 0; i < 500; i++) {
        var j = Math.floor(Math.random() * qtest_data.length);
        data.push({ url: qtest_data[j].url, expected: qtest_data[j].expected });
      }
      return data;
    }

    // This test exists to stress-test the caching mechanism in
    // UserAgentOverrideSet
    function test_UserAgentOverrideSetCaching(data) {
      webView.url = data.url;
      verify(webView.waitForLoadSucceeded());

      compare(webView.getTestApi().evaluateCode("return navigator.userAgent", true),
              data.expected);
    }
  }
}
