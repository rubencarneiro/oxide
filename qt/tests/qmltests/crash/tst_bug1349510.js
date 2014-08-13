exports.onGetUserAgentOverride = function(data) {
  data.userAgentOverride = "Foo";
  oxide.sendMessage({url: data.url});
}
