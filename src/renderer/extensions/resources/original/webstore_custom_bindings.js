// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Custom binding for the webstore API.

var webstoreNatives = requireNative('webstore');
var Event = require('event_bindings').Event;

function Installer() {
  this._pendingInstall = null;
  this.onInstallStageChanged =
      new Event(null, [{name: 'stage', type: 'string'}], {unmanaged: true});
  this.onDownloadProgress =
      new Event(null, [{name: 'progress', type: 'number'}], {unmanaged: true});
}

Installer.prototype.install = function(url, onSuccess, onFailure) {
  if (this._pendingInstall)
    throw new Error('A Chrome Web Store installation is already pending.');
  if (url !== undefined && typeof(url) !== 'string') {
    throw new Error(
        'The Chrome Web Store item link URL parameter must be a string.');
  }
  if (onSuccess !== undefined && typeof(onSuccess) !== 'function')
    throw new Error('The success callback parameter must be a function.');
  if (onFailure !== undefined && typeof(onFailure) !== 'function')
    throw new Error('The failure callback parameter must be a function.');

  // Since we call Install() with a bool for if we have listeners, listeners
  // must be set prior to the inline installation starting (this is also
  // noted in the Event documentation in
  // chrome/common/extensions/api/webstore.json).
  var installId = webstoreNatives.Install(
      this.onInstallStageChanged.hasListeners(),
      this.onDownloadProgress.hasListeners(),
      url,
      onSuccess,
      onFailure);
  if (installId !== undefined) {
    this._pendingInstall = {
      installId: installId,
      onSuccess: onSuccess,
      onFailure: onFailure
    };
  }
};

Installer.prototype.onInstallResponse = function(installId, success, error) {
  var pendingInstall = this._pendingInstall;
  if (!pendingInstall || pendingInstall.installId != installId) {
    // TODO(kalman): should this be an error?
    return;
  }

  try {
    if (success && pendingInstall.onSuccess)
      pendingInstall.onSuccess();
    else if (!success && pendingInstall.onFailure)
      pendingInstall.onFailure(error);
  } catch (e) {
    console.error('Exception in chrome.webstore.install response handler: ' +
                  e.stack);
  } finally {
    this._pendingInstall = null;
  }
};

Installer.prototype.onInstallStageChanged = function(installStage) {
  this.onInstallStageChanged.dispatch(installStage);
};

Installer.prototype.onDownloadProgress = function(progress) {
  this.onDownloadProgress.dispatch(progress);
};

var installer = new Installer();

var chromeWebstore = {
  install: function (url, onSuccess, onFailure) {
    installer.install(url, onSuccess, onFailure);
  },
  onInstallStageChanged: installer.onInstallStageChanged,
  onDownloadProgress: installer.onDownloadProgress
};

// This must match the name in InstallWebstoreBindings in
// chrome/renderer/extensions/dispatcher.cc.
exports.chromeWebstore = chromeWebstore;

// Called by webstore_bindings.cc.
exports.onInstallResponse =
    Installer.prototype.onInstallResponse.bind(installer);
exports.onInstallStageChanged =
    Installer.prototype.onInstallStageChanged.bind(installer);
exports.onDownloadProgress =
    Installer.prototype.onDownloadProgress.bind(installer);
