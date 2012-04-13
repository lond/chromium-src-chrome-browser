// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/ui_account_tweaks.h"

#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "chrome/browser/chromeos/cros_settings.h"
#include "chrome/browser/chromeos/cros_settings_names.h"
#include "chrome/browser/chromeos/login/user_manager.h"

namespace chromeos {

void AddAccountUITweaksLocalizedValues(
    base::DictionaryValue* localized_strings) {
  DCHECK(localized_strings);

  std::string owner_email;
  CrosSettings::Get()->GetString(kDeviceOwner, &owner_email);
  // Translate owner's email to the display email.
  std::string display_email =
      UserManager::Get()->GetUserDisplayEmail(owner_email);
  localized_strings->SetString("ownerUserId", UTF8ToUTF16(display_email));

  localized_strings->SetString("currentUserIsOwner",
      UserManager::Get()->IsCurrentUserOwner() ?
      ASCIIToUTF16("true") : ASCIIToUTF16("false"));

  localized_strings->SetString("loggedInAsGuest",
      chromeos::UserManager::Get()->IsLoggedInAsGuest() ?
      ASCIIToUTF16("true") : ASCIIToUTF16("false"));
}

void AddAccountUITweaksLocalizedValues(
    ChromeWebUIDataSource* source) {
  DCHECK(source);
  DictionaryValue dict;
  AddAccountUITweaksLocalizedValues(&dict);
  source->AddLocalizedStrings(dict);
}

}  // namespace chromeos
