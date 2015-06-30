import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.9
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  property var lastRequest
  property int lastStatusCode: -1
  property int totalRequests: 0
  property int expectedRequests: 0
  property int totalCancellations: 0
  property int expectedCancellations: 0
  property bool ignoreRequests: false

  // Since the browser caches credentials after the first successful
  // authentication, we make sure to use a different username for every test
  property string baseUrl: "http://testsuite/tst_httpAuthentication.py"
  property int userNameSuffix: 0
  property string currentUser: "user" + userNameSuffix
  property string credentialsUrl: baseUrl + "?" + currentUser + "_pass"

  onHttpAuthenticationRequested: {
      if (!ignoreRequests) {
          lastRequest = request
          lastRequest.cancelled.connect(updateCancellations)
      }
      totalRequests++
  }

  onLoadEvent: lastStatusCode = event.httpStatusCode

  function updateCancellations() { totalCancellations++ }

  TestCase {
    id: test
    name: "httpAuthentication"
    when: windowShown

    function init() {
        ignoreRequests = false
        if (lastRequest) {
            lastRequest.cancelled.disconnect(updateCancellations)
            lastRequest = null
        }
        webView.url = "about:blank";
        verify(webView.waitForLoadSucceeded());
        totalRequests = 0
        expectedRequests = 0
        totalCancellations = 0
        expectedCancellations = 0
        lastStatusCode = -1
        userNameSuffix++
    }

    function receivedRequest() {
        return totalRequests == expectedRequests
    }

    function receivedCancellation() {
        return totalCancellations == expectedCancellations
    }

    function test_requestMembers() {
        expectedRequests++
        webView.url = credentialsUrl
        verify(TestUtils.waitFor(receivedRequest), "No authentication request")
        compare(lastRequest.realm, "Fake Realm")
        compare(lastRequest.host, "testsuite:80")
    }

    function test_wrong_password() {
        expectedRequests++
        webView.url = credentialsUrl
        verify(TestUtils.waitFor(receivedRequest), "No authentication request")
        expectedRequests++
        lastRequest.allow(currentUser, "wrong")
        verify(TestUtils.waitFor(receivedRequest), "No authentication request")
    }

    function test_wrong_username() {
        expectedRequests++
        webView.url = credentialsUrl
        verify(TestUtils.waitFor(receivedRequest), "No authentication request")
        expectedRequests++
        lastRequest.allow(currentUser + "_wrong", "pass")
        verify(TestUtils.waitFor(receivedRequest), "No authentication request")
    }

    function test_right_credentials() {
        expectedRequests++
        webView.url = credentialsUrl
        verify(TestUtils.waitFor(receivedRequest), "No authentication request")
        lastRequest.allow(currentUser, "pass")
        webView.waitForLoadSucceeded()
        compare(lastStatusCode, 200)
        compare(expectedRequests, totalRequests)
    }

    function test_explicit_cancellation() {
        webView.url = credentialsUrl
        expectedRequests++
        verify(TestUtils.waitFor(receivedRequest), "No authentication request")

        lastRequest.deny()
        webView.waitForLoadSucceeded()
        compare(lastStatusCode, 401)
        // when cancelling explicity we should not receive cancellation signals
        compare(totalCancellations, 0)
    }

    function test_cancel_by_navigation() {
        webView.url = credentialsUrl
        expectedRequests++
        verify(TestUtils.waitFor(receivedRequest), "No authentication request")

        expectedCancellations++
        webView.url = "about:blank"
        verify(TestUtils.waitFor(receivedCancellation),
               "Authentication not cancelled")

        webView.waitForLoadSucceeded()
        compare(expectedRequests, totalRequests)
    }

    function test_ignored_request_cancelled() {
        ignoreRequests = true
        webView.url = credentialsUrl
        expectedRequests++
        verify(TestUtils.waitFor(receivedRequest), "No authentication request")

        // force garbage collection so the request object will be destroyed
        // as we did not keep any reference to it.
        // the request destructor will call request.deny()
        gc()
        webView.waitForLoadSucceeded()
        compare(lastStatusCode, 401)
    }
  }
}
