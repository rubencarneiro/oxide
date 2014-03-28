import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
    id: webView
    focus: true
    width: 200
    height: 200

    SignalSpy {
        id: spy
        target: webView
        signalName: "navigationHistoryChanged"
    }

    SignalSpy {
        id: urlSpy
        target: webView
        signalName: "urlChanged"
    }

    onNavigationRequested: {
        request.accept = false;
    }

    TestCase {
        id: test
        name: "WebView_navigation_request"
        when: windowShown

        function init() {
            spy.clear();
        }

        readonly property var initData: [
            { url: "http://localhost:8080/tst_WebView_navigation_request1.html", index: 0 }
        ]

        function test_WebView_navigation_request1_init_data() {
            return initData;
        }

        function test_WebView_navigation_request1_init(data) {
            webView.url = data.url;

            verify(webView.waitForLoadSucceeded(),
                   "Timed out waiting for load to finish");

            compare(webView.url, data.url,
                    "WebView.url is invalid after load");
            webView.waitForLoadSucceeded();
            compare(webView.url, data.url,
                    "navigation request should be ignored");
        }

    }
}
