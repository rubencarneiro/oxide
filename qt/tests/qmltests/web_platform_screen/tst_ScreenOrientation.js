window.addEventListener("orientationchange", function(e) {
  oxide.sendMessage("ORIENTATION-EVENT");
});
