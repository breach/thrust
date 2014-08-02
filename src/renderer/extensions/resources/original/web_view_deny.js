// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var DocumentNatives = requireNative('document_natives');

// Output error message to console when using the <webview> tag with no
// permission.
var errorMessage = "You do not have permission to use the webview element." +
  " Be sure to declare the 'webview' permission in your manifest file.";

// Registers <webview> custom element.
function registerWebViewElement() {
  var proto = Object.create(HTMLElement.prototype);

  proto.createdCallback = function() {
    console.error(errorMessage);
  };

  window.WebView =
      DocumentNatives.RegisterElement('webview', {prototype: proto});

  // Delete the callbacks so developers cannot call them and produce unexpected
  // behavior.
  delete proto.createdCallback;
  delete proto.enteredDocumentCallback;
  delete proto.leftDocumentCallback;
  delete proto.attributeChangedCallback;
}

var useCapture = true;
window.addEventListener('readystatechange', function listener(event) {
  if (document.readyState == 'loading')
    return;

  registerWebViewElement();
  window.removeEventListener(event.type, listener, useCapture);
}, useCapture);
