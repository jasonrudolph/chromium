// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @implements {nux.NuxSetAsDefaultProxy} */
class TestNuxSetAsDefaultProxy extends TestBrowserProxy {
  constructor() {
    super([
      'requestDefaultBrowserState',
      'setAsDefault',
    ]);

    this.defaultStatus_ = {};
  }

  /** @override */
  requestDefaultBrowserState() {
    cr.webUIListenerCallback(
        'browser-default-state-changed', this.defaultStatus_);
    this.methodCalled('requestDefaultBrowserState');
  }

  /** @override */
  setAsDefault() {
    this.methodCalled('setAsDefault');
  }

  /** @param {!nux.DefaultBrowserInfo} status */
  setDefaultStatus(status) {
    this.defaultStatus_ = status;
  }
}
