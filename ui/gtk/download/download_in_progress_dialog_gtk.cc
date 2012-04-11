// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/gtk/download/download_in_progress_dialog_gtk.h"

#include <gtk/gtk.h>

#include "base/string16.h"
#include "base/string_number_conversions.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/gtk/gtk_util.h"
#include "grit/chromium_strings.h"
#include "grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"

// static
void DownloadInProgressDialogGtk::Show(Browser* browser,
                                       gfx::NativeWindow parent_window) {
  new DownloadInProgressDialogGtk(browser, parent_window);
}

DownloadInProgressDialogGtk::DownloadInProgressDialogGtk(
    Browser* browser,
    gfx::NativeWindow parent_window)
    : browser_(browser) {
  int download_count;
  Browser::DownloadClosePreventionType type =
      browser_->OkToCloseWithInProgressDownloads(&download_count);

  // This dialog should have been created within the same thread invocation
  // as the original test that lead to us, so it should always not be ok
  // to close.
  DCHECK_NE(Browser::DOWNLOAD_CLOSE_OK, type);

  // TODO(rdsmith): This dialog should be different depending on whether we're
  // closing the last incognito window of a profile or doing browser shutdown.
  // See http://crbug.com/88421.

  std::string warning_text;
  std::string explanation_text;
  std::string ok_button_text;
  std::string cancel_button_text;
  if (download_count == 1) {
    warning_text = l10n_util::GetStringUTF8(
        IDS_SINGLE_DOWNLOAD_REMOVE_CONFIRM_WARNING);
    explanation_text = l10n_util::GetStringUTF8(
        IDS_SINGLE_DOWNLOAD_REMOVE_CONFIRM_EXPLANATION);
    ok_button_text = l10n_util::GetStringUTF8(
        IDS_SINGLE_DOWNLOAD_REMOVE_CONFIRM_OK_BUTTON_LABEL);
    cancel_button_text = l10n_util::GetStringUTF8(
        IDS_SINGLE_DOWNLOAD_REMOVE_CONFIRM_CANCEL_BUTTON_LABEL);
  } else {
    warning_text = l10n_util::GetStringFUTF8(
        IDS_MULTIPLE_DOWNLOADS_REMOVE_CONFIRM_WARNING,
        base::IntToString16(download_count));
    explanation_text = l10n_util::GetStringUTF8(
        IDS_MULTIPLE_DOWNLOADS_REMOVE_CONFIRM_EXPLANATION);
    ok_button_text = l10n_util::GetStringUTF8(
        IDS_MULTIPLE_DOWNLOADS_REMOVE_CONFIRM_OK_BUTTON_LABEL);
    cancel_button_text = l10n_util::GetStringUTF8(
        IDS_MULTIPLE_DOWNLOADS_REMOVE_CONFIRM_CANCEL_BUTTON_LABEL);
  }

  GtkWidget* dialog = gtk_message_dialog_new(
      parent_window,
      static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL),
      GTK_MESSAGE_QUESTION,
      GTK_BUTTONS_NONE,
      "%s",
      warning_text.c_str());
  gtk_util::AddButtonToDialog(dialog, cancel_button_text.c_str(),
                              GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT);
  gtk_util::AddButtonToDialog(dialog, ok_button_text.c_str(),
                              GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);

  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
                                           "%s",
                                           explanation_text.c_str());

  g_signal_connect(dialog, "response", G_CALLBACK(OnResponseThunk), this);

  gtk_widget_show_all(dialog);
}

DownloadInProgressDialogGtk::~DownloadInProgressDialogGtk() {
}

void DownloadInProgressDialogGtk::OnResponse(GtkWidget* dialog,
                                             int response_id) {
  gtk_widget_destroy(dialog);
  browser_->InProgressDownloadResponse(response_id == GTK_RESPONSE_ACCEPT);
  delete this;
}
