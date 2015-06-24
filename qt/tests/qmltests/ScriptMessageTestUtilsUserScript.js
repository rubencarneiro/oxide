// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// ==UserScript==
// @run-at document-start
// ==/UserScript==

oxide.addMessageHandler("SEND-MESSAGE-TO-BROWSER", function(msg) {
  var r = oxide.sendMessage(msg.payload.id, msg.payload.payload);
  r.onreply = function(response) {
    msg.reply({error: 0, response: response});
  };
  r.onerror = function(error, response) {
    msg.reply({error: error, response: response});
  };
});

oxide.addMessageHandler("TEST-GENERATE-JS-EXCEPTION", function(msg) {
  throw Exception("This is an error");
});

oxide.addMessageHandler("TEST-DONT-RESPOND", function(msg) {});

oxide.addMessageHandler("TEST-REPLY", function(msg) {
  msg.reply(eval(msg.payload));
});

oxide.addMessageHandler("TEST-ERROR", function(msg) {
  msg.error(msg.payload);
});

oxide.addMessageHandler("TEST-SEND-MESSAGE-NO-REPLY", function(msg) {
  oxide.sendMessage(msg.id + "-RESPONSE", msg.payload);
});
