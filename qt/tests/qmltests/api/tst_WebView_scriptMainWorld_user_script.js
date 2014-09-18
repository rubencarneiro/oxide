// ==UserScript==
// @inject_in_main_world
// ==/UserScript==

try {
  oxide.sendMessage("from-user-script", {"data": "mydata", "values": [1, 2, 3]});
  document.getElementById("result").innerHTML =
	"Main world content script found oxide.sendMessage";
} catch(e) {
  document.getElementById("result").innerHTML =
	"Main world content script DID NOT found oxide.sendMessage";
}

