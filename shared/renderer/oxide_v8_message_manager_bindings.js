// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

const TYPE_MESSAGE = 0;
const TYPE_REPLY = 1;

const ERROR_OK = 0;
const ERROR_NO_HANDLER = 3;
const ERROR_HANDLER_REPORTED_ERROR = 4;

var g_serial = 0;
var g_requests = {};
var g_handlers = {};

function getNextSerial() {
  // Make sure serial fits in to an int32
  while (++g_serial % 0x80000000 in g_requests) {};
  return g_serial % 0x80000000;
}

function SendMessageRequest() {}

SendMessageRequest.prototype = Object.create(Object.prototype, {
  onreply: { writable: true },
  onerror: { writable: true }
});

function createIncomingMessage(serial) {
  return Object.create(Object.prototype, {
    reply: { value: function(args) {
      sendMessageNative(serial, TYPE_REPLY, ERROR_OK, "", JSON.stringify(args));
    }},

    error: { value: function(msg) {
      sendMessageNative(serial, TYPE_REPLY, ERROR_HANDLER_REPORTED_ERROR, "", msg);
    }}
  });
}

registerReceiveHandlerNative(function messageHandler(serial, type, error, id, args) {
  if (type == TYPE_REPLY) {
    if (!(serial in g_requests)) {
      return;
    }

    var r = g_requests[serial];
    delete g_requests[serial];

    if (error == ERROR_OK && typeof(r.onreply) == "function") {
      r.onreply.call(null, JSON.parse(args));
    } else if (typeof(r.onerror) == "function") {
      r.onerror.call(null, error, args);
    }
  } else {
    if (!(id in g_handlers)) {
      sendMessageNative(serial, TYPE_REPLY, ERROR_NO_HANDLER, "",
                        "No registered handler for \"" + id + "\" message");
      return;
    }

    var msg = createIncomingMessage(serial);
    msg.args = JSON.parse(args);

    g_handlers[id].call(null, msg);
  }
});

exports.sendMessage = function(id, args) {
  var serial = getNextSerial();

  sendMessageNative(serial, TYPE_MESSAGE, ERROR_OK, id, JSON.stringify(args));

  g_requests[serial] = new SendMessageRequest();
  return g_requests[serial];
};

exports.sendMessageNoReply = function(id, args) {
  sendMessageNative(-1, TYPE_MESSAGE, ERROR_OK, id, JSON.stringify(args));
};

exports.addMessageHandler = function(id, handler) {
  if (id in g_handlers) {
    throw new Error("Handler already registered for this message ID");
  }

  g_handlers[id] = handler;
};

exports.removeMessageHandler = function(id, handler) {
  if (!(id in g_handlers) || g_handlers[id] != handler) {
    throw new Error("Handler not registered");
  }

  delete g_handlers[id];
};
