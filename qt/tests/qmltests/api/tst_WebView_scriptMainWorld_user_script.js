try {
  oxide.sendMessage("from-user-script", {"data": "mydata", "values": [1, 2, 3]});
  oxide.addMessageHandler("from-user-script", function(msg) {});
  document.getElementById("result").innerHTML =
	"Main world content script found oxide.sendMessage";
} catch(e) {
  document.getElementById("result").innerHTML =
	"Main world content script DID NOT found oxide.sendMessage";
}

