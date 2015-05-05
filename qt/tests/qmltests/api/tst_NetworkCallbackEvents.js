var test = null;

oxide.onMessage = function(msg) {
  test = msg.test;
};

exports.onBeforeURLRequest = function(event) {
  if (test == "beforeURLRequest") {
    oxide.sendMessage({ url: event.url, method: event.method,
                        requestCancelled: event.requestCancelled,
                        redirectUrl: event.redirectUrl,
                        isMainFrame: event.isMainFrame });

    if (event.url == "http://testsuite/empty.html" && event.isMainFrame) {
      event.redirectUrl = "http://testsuite/tst_NetworkCallbackEvents1.html";
    } else if (event.url == "http://testsuite/tst_NetworkCallbackEvents1_foo.html") {
      event.redirectUrl = "http://testsuite/empty.html";
    }
  } else if (test == "cancelBeforeURLRequest" && event.isMainFrame) {
    event.cancelRequest();
    oxide.sendMessage({ requestCancelled: event.requestCancelled });
  } else if (test == "cancelBeforeURLRequest_SubFrame" && !event.isMainFrame) {
    event.cancelRequest();
  } else if (test == "referrer") {
    oxide.sendMessage({ referrer: event.referrer });
  }
};

exports.onBeforeSendHeaders = function(event) {
  if (test == "beforeSendHeaders") {
    oxide.sendMessage({ url: event.url, method: event.method,
                        requestCancelled: event.requestCancelled,
                        isMainFrame: event.isMainFrame,
                        hasUA: event.hasHeader("User-Agent"),
                        UA: event.getHeader("User-Agent"),
                        hasFoo: event.hasHeader("Foo"),
                        Foo: event.getHeader("Foo") });

    if (event.url == "http://testsuite/get-headers.py?override-ua") {
      event.setHeader("User-Agent", "Bleurgh");
    } else if (event.url == "http://testsuite/get-headers.py?add-foo") {
      event.setHeader("Foo", "Bar");
    }
  } else if (test == "cancelBeforeSendHeaders" && event.isMainFrame) {
    event.cancelRequest();
    oxide.sendMessage({ requestCancelled: event.requestCancelled });
  } else if (test == "cancelBeforeSendHeaders_SubFrame" && !event.isMainFrame) {
    event.cancelRequest();
  } else if (test == "referrer") {
    oxide.sendMessage({ referrer: event.referrer });
  }
}

exports.onBeforeRedirect = function(event) {
  if (test == "beforeRedirect") {
    oxide.sendMessage({ url: event.url, method: event.method,
                        requestCancelled: event.requestCancelled,
                        originalUrl: event.originalUrl,
                        isMainFrame: event.isMainFrame });
  } else if (test == "cancelBeforeRedirect" && event.isMainFrame) {
    event.cancelRequest();
    oxide.sendMessage({ requestCancelled: event.requestCancelled });
  } else if (test == "cancelBeforeRedirect_SubFrame" && !event.isMainFrame) {
    event.cancelRequest();
  } else if (test == "referrer") {
    oxide.sendMessage({ referrer: event.referrer });
  }
}
