try {
  oxide.sendMessage("from-user-script", {"data": "mydata", "values": [1, 2, 3]});
  document.getElementById("result").innerHTML =
	"Main world content script found oxide.postMessage";
} catch(e) {
  document.getElementById("result").innerHTML =
	"Main world content script DID NOT found oxide.postMessage";
}

