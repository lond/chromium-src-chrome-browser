// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/login/managed/locally_managed_user_creation_screen.h"

#include "base/values.h"
#include "chrome/browser/chromeos/login/existing_user_controller.h"
#include "chrome/browser/chromeos/login/managed/locally_managed_user_creation_controller.h"
#include "chrome/browser/chromeos/login/screens/error_screen.h"
#include "chrome/browser/chromeos/login/screens/screen_observer.h"
#include "chrome/browser/chromeos/login/wizard_controller.h"
#include "chromeos/network/network_state.h"
#include "grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"

namespace chromeos {

namespace {

void ConfigureErrorScreen(ErrorScreen* screen,
    const NetworkState* network,
    const NetworkPortalDetector::CaptivePortalStatus status) {
  switch (status) {
    case NetworkPortalDetector::CAPTIVE_PORTAL_STATUS_UNKNOWN:
    case NetworkPortalDetector::CAPTIVE_PORTAL_STATUS_ONLINE:
      NOTREACHED();
      break;
    case NetworkPortalDetector::CAPTIVE_PORTAL_STATUS_OFFLINE:
      screen->SetErrorState(ErrorScreen::ERROR_STATE_OFFLINE,
                            std::string());
      break;
    case NetworkPortalDetector::CAPTIVE_PORTAL_STATUS_PORTAL:
      screen->SetErrorState(ErrorScreen::ERROR_STATE_PORTAL,
                            network->name());
      screen->FixCaptivePortal();
      break;
    case NetworkPortalDetector::CAPTIVE_PORTAL_STATUS_PROXY_AUTH_REQUIRED:
      screen->SetErrorState(ErrorScreen::ERROR_STATE_PROXY,
                            std::string());
      break;
    default:
      NOTREACHED();
      break;
  }
}

} // namespace

LocallyManagedUserCreationScreen::LocallyManagedUserCreationScreen(
    ScreenObserver* observer,
    LocallyManagedUserCreationScreenHandler* actor)
    : WizardScreen(observer),
      actor_(actor),
      on_error_screen_(false),
      on_image_screen_(false) {
  DCHECK(actor_);
  if (actor_)
    actor_->SetDelegate(this);
}

LocallyManagedUserCreationScreen::~LocallyManagedUserCreationScreen() {
  if (actor_)
    actor_->SetDelegate(NULL);
}

void LocallyManagedUserCreationScreen::PrepareToShow() {
  if (actor_)
    actor_->PrepareToShow();
}

void LocallyManagedUserCreationScreen::Show() {
  if (actor_) {
    actor_->Show();
    // TODO(antrim) : temorary hack (until upcoming hackaton). Should be
    // removed once we have screens reworked.
    if (on_image_screen_)
      actor_->ShowTutorialPage();
    else
      actor_->ShowIntroPage();
  }

  NetworkPortalDetector* detector = NetworkPortalDetector::GetInstance();
  if (detector && !on_error_screen_)
    detector->AddAndFireObserver(this);
  on_error_screen_ = false;
}

void LocallyManagedUserCreationScreen::OnPortalDetectionCompleted(
    const NetworkState* network,
    const NetworkPortalDetector::CaptivePortalState& state)  {
  if (state.status == NetworkPortalDetector::CAPTIVE_PORTAL_STATUS_ONLINE) {
    get_screen_observer()->HideErrorScreen(this);
  } else {
    on_error_screen_ = true;
    ErrorScreen* screen = get_screen_observer()->GetErrorScreen();
    ConfigureErrorScreen(screen, network, state.status);
    screen->SetUIState(ErrorScreen::UI_STATE_LOCALLY_MANAGED);
    get_screen_observer()->ShowErrorScreen();
  }
}

void LocallyManagedUserCreationScreen::
    ShowManagerInconsistentStateErrorScreen() {
  if (!actor_)
    return;
  actor_->ShowErrorPage(
      l10n_util::GetStringUTF16(
          IDS_CREATE_LOCALLY_MANAGED_USER_MANAGER_INCONSISTENT_STATE));
}

void LocallyManagedUserCreationScreen::ShowInitialScreen() {
  if (actor_)
    actor_->ShowIntroPage();
}

void LocallyManagedUserCreationScreen::Hide() {
  if (actor_)
    actor_->Hide();
  NetworkPortalDetector* detector = NetworkPortalDetector::GetInstance();
  if (detector && !on_error_screen_)
    detector->RemoveObserver(this);
}

std::string LocallyManagedUserCreationScreen::GetName() const {
  return WizardController::kLocallyManagedUserCreationScreenName;
}

void LocallyManagedUserCreationScreen::AbortFlow() {
  controller_->CancelCreation();
}

void LocallyManagedUserCreationScreen::FinishFlow() {
  controller_->FinishCreation();
}

void LocallyManagedUserCreationScreen::AuthenticateManager(
    const std::string& manager_id,
    const std::string& manager_password) {
  // Make sure no two controllers exist at the same time.
  controller_.reset();
  controller_.reset(new LocallyManagedUserCreationController(this, manager_id));

  ExistingUserController::current_controller()->
      Login(UserContext(manager_id,
                        manager_password,
                        std::string()  /* auth_code */));
}

void LocallyManagedUserCreationScreen::CreateManagedUser(
    const string16& display_name,
    const std::string& managed_user_password) {
  DCHECK(controller_.get());
  controller_->SetUpCreation(display_name, managed_user_password);
  controller_->StartCreation();
}

void LocallyManagedUserCreationScreen::OnManagerLoginFailure() {
  if (actor_)
    actor_->ShowManagerPasswordError();
}

void LocallyManagedUserCreationScreen::OnManagerFullyAuthenticated(
    Profile* manager_profile) {
  DCHECK(controller_.get());
  controller_->SetManagerProfile(manager_profile);
  if (actor_)
    actor_->ShowUsernamePage();
}

void LocallyManagedUserCreationScreen::OnManagerCryptohomeAuthenticated() {
  if (actor_) {
    actor_->ShowStatusMessage(true /* progress */, l10n_util::GetStringUTF16(
            IDS_CREATE_LOCALLY_MANAGED_USER_CREATION_AUTH_PROGRESS_MESSAGE));
  }
}

void LocallyManagedUserCreationScreen::OnExit() {}

void LocallyManagedUserCreationScreen::OnActorDestroyed(
    LocallyManagedUserCreationScreenHandler* actor) {
  if (actor_ == actor)
    actor_ = NULL;
}

void LocallyManagedUserCreationScreen::OnCreationError(
    LocallyManagedUserCreationController::ErrorCode code) {
  string16 message;
  // TODO(antrim) : find out which errors do we really have.
  // We might reuse some error messages from ordinary user flow.
  switch (code) {
    case LocallyManagedUserCreationController::CRYPTOHOME_NO_MOUNT:
    case LocallyManagedUserCreationController::CRYPTOHOME_FAILED_MOUNT:
    case LocallyManagedUserCreationController::CRYPTOHOME_FAILED_TPM:
      message = l10n_util::GetStringUTF16(
          IDS_CREATE_LOCALLY_MANAGED_USER_CREATION_ERROR_TPM_ERROR);
      break;
    case LocallyManagedUserCreationController::CLOUD_SERVER_ERROR:
      message = l10n_util::GetStringUTF16(
          IDS_CREATE_LOCALLY_MANAGED_USER_CREATION_ERROR_SERVER_ERROR);
      break;
    case LocallyManagedUserCreationController::TOKEN_WRITE_FAILED:
      message = l10n_util::GetStringUTF16(
          IDS_CREATE_LOCALLY_MANAGED_USER_CREATION_ERROR_TOKEN_WRITE_ERROR);
      break;
    case LocallyManagedUserCreationController::NO_ERROR:
      NOTREACHED();
  }
  if (actor_)
    actor_->ShowErrorPage(message);
}

void LocallyManagedUserCreationScreen::SelectPicture() {
  on_image_screen_ = true;
  WizardController::default_controller()->
      EnableUserImageScreenReturnToPreviousHack();
  DictionaryValue* params = new DictionaryValue();
  params->SetBoolean("profile_picture_enabled", false);
  params->SetString("user_id", controller_->GetManagedUserId());

  WizardController::default_controller()->
      AdvanceToScreenWithParams(WizardController::kUserImageScreenName, params);
}

void LocallyManagedUserCreationScreen::OnCreationSuccess() {
  SelectPicture();
}

void LocallyManagedUserCreationScreen::OnCreationTimeout() {
  if (actor_) {
    actor_->ShowStatusMessage(false /* error */, l10n_util::GetStringUTF16(
        IDS_CREATE_LOCALLY_MANAGED_USER_CREATION_CREATION_TIMEOUT_MESSAGE));
  }
}

}  // namespace chromeos
