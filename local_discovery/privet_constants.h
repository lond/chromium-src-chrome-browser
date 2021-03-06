// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_LOCAL_DISCOVERY_PRIVET_CONSTANTS_H_
#define CHROME_BROWSER_LOCAL_DISCOVERY_PRIVET_CONSTANTS_H_

namespace local_discovery {

extern const char kPrivetKeyError[];
extern const char kPrivetInfoKeyToken[];
extern const char kPrivetKeyDeviceID[];
extern const char kPrivetKeyClaimURL[];
extern const char kPrivetKeyClaimToken[];
extern const char kPrivetKeyTimeout[];

extern const char kPrivetErrorDeviceBusy[];
extern const char kPrivetErrorPendingUserAction[];
extern const char kPrivetErrorInvalidXPrivetToken[];
extern const char kPrivetErrorTimeout[];
extern const char kPrivetErrorCancel[];

extern const char kPrivetActionStart[];
extern const char kPrivetActionGetClaimToken[];
extern const char kPrivetActionComplete[];
extern const char kPrivetActionCancel[];

// Name for pseudo-action "info", used only to show info stage in errors.
extern const char kPrivetActionNameInfo[];

extern const char kPrivetDefaultDeviceType[];
extern const char kPrivetSubtypeTemplate[];

const double kPrivetMaximumTimeScaling = 1.2;

extern const char kPrivetTxtKeyName[];
extern const char kPrivetTxtKeyDescription[];
extern const char kPrivetTxtKeyURL[];
extern const char kPrivetTxtKeyType[];
extern const char kPrivetTxtKeyID[];
extern const char kPrivetTxtKeyConnectionState[];

extern const char kPrivetConnectionStatusOnline[];
extern const char kPrivetConnectionStatusOffline[];
extern const char kPrivetConnectionStatusConnecting[];
extern const char kPrivetConnectionStatusNotConfigured[];

const int kPrivetDefaultTimeout = 15;

const double kPrivetMaximumTimeRandomAddition = 0.2;

const int kAccountIndexUseOAuth2 = -1;

}  // namespace local_discovery

#endif  // CHROME_BROWSER_LOCAL_DISCOVERY_PRIVET_CONSTANTS_H_
