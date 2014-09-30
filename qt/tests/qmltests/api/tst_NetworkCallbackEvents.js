exports.onBeforeURLRequest = function(event) {
  oxide.sendMessage({ event: "beforeURLRequest",
                      url: event.url, method: event.method,
                      requestCancelled: event.requestCancelled,
                      redirectUrl: event.redirectUrl });

  if (event.url == "http://testsuite/tst_NetworkCallbackEvents2.html") {
    event.redirectUrl = "http://testsuite/empty.html";
  } else if (event.url == "http://testsuite/tst_NetworkCallbackEvents3.html") {
    event.cancelRequest();
  }
};

exports.onBeforeSendHeaders = function(event) {
  oxide.sendMessage({ event: "beforeSendHeaders",
                      url: event.url, method: event.method,
                      requestCancelled: event.requestCancelled,
                      hasUA: event.hasHeader("User-Agent"),
                      UA: event.getHeader("User-Agent"),
                      hasFoo: event.hasHeader("Foo"),
                      Foo: event.getHeader("Foo") });

  if (event.url == "http://testsuite/get-headers.py?override-ua") {
    event.setHeader("User-Agent", "Bleurgh");
  } else if (event.url == "http://testsuite/get-headers.py?clear-ua") {
    event.clearHeader("User-Agent");
  } else if (event.url == "http://testsuite/get-headers.py?add-foo") {
    event.setHeader("Foo", "Bar");
  }
}

exports.onBeforeRedirect = function(event) {
  if (event.url == "http://localhost:8080/redirect.py?cancel") {
    event.cancelRequest();
  }

  oxide.sendMessage({ event: "onBeforeRedirect",
                      url: event.url, method: event.method,
                      requestCancelled: event.requestCancelled,
                      newUrl: event.newUrl,
                      isMainFrame: event.isMainFrame,
                      httpResponseCode: event.httpResponseCode });
}
