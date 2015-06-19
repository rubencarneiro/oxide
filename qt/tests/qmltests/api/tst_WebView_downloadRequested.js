exports.onGetUserAgentOverride = function(data) {
  if (data.url == "http://testsuite/tst_WebView_downloadRequested.py?override") {
    data.userAgentOverride = "Override download user agent string";
  }
}
