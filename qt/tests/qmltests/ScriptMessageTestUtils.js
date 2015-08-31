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

.pragma library
.import "TestUtils.js" as TestUtils

var kDefaultContextUrl = "oxide://script-message-test-utils/";

function init(webContext, scriptContext) {
  webContext.addTestUserScript({ context: scriptContext || kDefaultContextUrl,
                                 url: Qt.resolvedUrl("ScriptMessageTestUtilsUserScript.js"),
                                 incognitoEnabled: true, matchAllFrames: true });
}

function waitForResult(req, gcDuringWait, throwError) {
  var result;
  var error;

  req.onreply = function(response) {
    result = response;
    error = 0;
  };
  req.onerror = function(code, response) {
    result = response;
    error = code;
  };

  TestUtils.waitFor(function() { return error !== undefined; }, null, gcDuringWait);

  if (error > 0) {
    if (throwError) {
      throw new TestUtils.MessageError(error, result);
    }
    return new TestUtils.MessageError(error, result);
  } else if (error === 0) {
    return result;
  } else {
    throw new Error("Message call timed out");
  }
}

function FrameHelper(frame, context) {
  this._frame = frame;
  this._context = context || kDefaultContextUrl;
}

FrameHelper.prototype = {
  sendMessageToBrowser: function(id, payload, gcDuringWait) {
    var r = waitForResult(
        this._frame.sendMessage(this._context,
                                "SEND-MESSAGE-TO-BROWSER",
                                { id: id,
                                  payload: payload === undefined ? null : payload }),
        gcDuringWait, true);
    if (r.error > 0) {
      return new TestUtils.MessageError(r.error, r.response);
    } else {
      return r.response;
    }
  },

  sendMessageToBrowserNoWait: function(id, payload) {
    return this._frame.sendMessage(this._context,
                                   "SEND-MESSAGE-TO-BROWSER",
                                   { id: id,
                                     payload: payload === undefined ? null : payload });
  },

  sendMessage: function(id, payload) {
    return waitForResult(
        this._frame.sendMessage(this._context, id, payload), false, false);
  },

  sendMessageNoWait: function(id, payload) {
    return this._frame.sendMessage(this._context, id, payload);
  },

  sendMessageNoReply: function(id, payload) {
    this._frame.sendMessageNoReply(this._context, id, payload);
  }
};
