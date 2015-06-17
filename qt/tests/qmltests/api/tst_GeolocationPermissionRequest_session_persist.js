// ==UserScript==
// @run-at document-start
// ==/UserScript==

document.addEventListener("oxidegeolocationresult", function(event) {
  oxide.sendMessage("GEOLOCATION-RESPONSE", { status: event.detail.status });
});
