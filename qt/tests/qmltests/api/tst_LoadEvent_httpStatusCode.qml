import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.8
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  property var expectedLoadEvents: []
  property int expectedHttpStatusCode

  onLoadEvent: {
    test.verify(expectedLoadEvents.length > 0);
    var expected = expectedLoadEvents[0];
    expectedLoadEvents.shift();

    test.compare(event.type, expected, "Unexpected type")

    test.compare(event.httpStatusCode,
                 event.type != LoadEvent.TypeStarted && event.type != LoadEvent.TypeStopped ? expectedHttpStatusCode : 0,
                 "Unexpected value of httpStatusCode");
  }

  TestCase {
    id: test
    name: "LoadEvent_httpStatusCode"
    when: windowShown

    function init() {
      webView.clearLoadEventCounters();
      expectedLoadEvents = [];
    }

    function test_HttpStatusCode2xx_data() {
      return [
        {
          tag: "200",
          httpStatusCode: 200,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "201",
          httpStatusCode: 201,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "202",
          httpStatusCode: 202,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "203",
          httpStatusCode: 203,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "204",
          httpStatusCode: 204,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeStopped
          ]
        },
        {
          tag: "205",
          httpStatusCode: 205,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeStopped
          ]
        },
        {
          tag: "206",
          httpStatusCode: 206,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
      ]
    }

    function test_HttpStatusCode2xx(data) {
      expectedHttpStatusCode = data.httpStatusCode;
      expectedLoadEvents = data.events;

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?" +
        data.httpStatusCode;

      if (data.httpStatusCode === 204 || data.httpStatusCode === 205) {
        verify(webView.waitForLoadStopped());
      } else {
        verify(webView.waitForLoadSucceeded());
      }
    }

    function test_HttpStatusCode3xx_data() {
      return [
        {
          tag: "300",
          httpStatusCode: 300,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "301",
          httpStatusCode: 301,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "302",
          httpStatusCode: 302,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "303",
          httpStatusCode: 303,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "304",
          httpStatusCode: 304,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeStopped
          ]
        },
        {
          tag: "305",
          httpStatusCode: 305,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "306",
          httpStatusCode: 306,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "307",
          httpStatusCode: 307,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "308",
          httpStatusCode: 308,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        }
      ]
    }

    function test_HttpStatusCode3xx(data) {
      expectedHttpStatusCode = data.httpStatusCode;
      expectedLoadEvents = data.events;

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?" +
        data.httpStatusCode;

      if (data.httpStatusCode === 304) {
        verify(webView.waitForLoadStopped());
      } else {
        verify(webView.waitForLoadSucceeded());
      }
    }

    function test_HttpStatusCode4xx_data() {
      return [
        {
          tag: "400",
          httpStatusCode: 400,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "401",
          httpStatusCode: 401,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "403",
          httpStatusCode: 403,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "404",
          httpStatusCode: 404,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "405",
          httpStatusCode: 405,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "406",
          httpStatusCode: 406,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        // XXX: investigate on failure of this test.
        // See https://code.launchpad.net/~rpadovani/oxide/http-status-code/+merge/256606/comments/647631
        // {
        //   tag: "407",
        //   httpStatusCode: 407,
        //   events: [
        //     LoadEvent.TypeStarted,
        //     LoadEvent.TypeFailed,
        //     LoadEvent.TypeCommitted
        //   ]
        // },
        {
          tag: "408",
          httpStatusCode: 408,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "409",
          httpStatusCode: 409,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "410",
          httpStatusCode: 410,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "411",
          httpStatusCode: 411,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "412",
          httpStatusCode: 412,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "413",
          httpStatusCode: 413,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "414",
          httpStatusCode: 414,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "415",
          httpStatusCode: 415,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "416",
          httpStatusCode: 416,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "417",
          httpStatusCode: 417,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        }
      ]
    }

    function test_HttpStatusCode4xx(data) {
      expectedHttpStatusCode = data.httpStatusCode;
      expectedLoadEvents = data.events;

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?" +
        data.httpStatusCode;

      if (data.httpStatusCode === 407) {
        verify(webView.waitForLoadCommitted());
      } else {
        verify(webView.waitForLoadSucceeded());
      }
    }

    function test_HttpStatusCode5xx_data() {
      return [
        {
          tag: "500",
          httpStatusCode: 500,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "501",
          httpStatusCode: 501,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "502",
          httpStatusCode: 502,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "503",
          httpStatusCode: 503,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "504",
          httpStatusCode: 504,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        },
        {
          tag: "505",
          httpStatusCode: 505,
          events: [
            LoadEvent.TypeStarted,
            LoadEvent.TypeCommitted,
            LoadEvent.TypeSucceeded
          ]
        }
      ]
    }

    function test_HttpStatusCode5xx(data) {
      expectedHttpStatusCode = data.httpStatusCode;
      expectedLoadEvents = data.events;

      webView.url = "http://testsuite/tst_LoadEvent_httpStatusCode.py?" +
        data.httpStatusCode;

      // if (data.httpStatusCode === 304) {
      //   verify(webView.waitForLoadStopped());
      // } else {
        verify(webView.waitForLoadSucceeded());
      //}
    }
  }
}
