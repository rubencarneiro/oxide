exports.onGetUserAgentOverride = function(data) {
  if (data.url == "http://testsuite/empty.html?override") {
    data.userAgentOverride = "Override user agent string";
  }
}
