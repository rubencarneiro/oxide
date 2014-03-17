exports.onGetUserAgentOverride = function(data) {
  if (data.url == "http://localhost:8080/empty.html?override") {
    data.userAgentOverride = "Override user agent string";
  }
}
