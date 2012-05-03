// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_WEB_DIALOG_UI_H_
#define CHROME_BROWSER_UI_WEBUI_WEB_DIALOG_UI_H_
#pragma once

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/string16.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_ui_controller.h"
#include "googleurl/src/gurl.h"
#include "ui/base/ui_base_types.h"


namespace base {
class ListValue;
template<class T> class PropertyAccessor;
}

namespace content {
class WebContents;
class WebUIMessageHandler;
struct ContextMenuParams;
}

namespace gfx {
class Size;
}

class WebDialogDelegate;

// Displays file URL contents inside a modal web dialog.
//
// This application really should not use WebContents + WebUI. It should instead
// just embed a RenderView in a dialog and be done with it.
//
// Before loading a URL corresponding to this WebUI, the caller should set its
// delegate as a property on the WebContents. This WebUI will pick it up from
// there and call it back. This is a bit of a hack to allow the dialog to pass
// its delegate to the Web UI without having nasty accessors on the WebContents.
// The correct design using RVH directly would avoid all of this.
class WebDialogUI : public content::WebUIController {
 public:
  struct WebDialogParams {
    // The URL for the content that will be loaded in the dialog.
    GURL url;
    // Width of the dialog.
    int width;
    // Height of the dialog.
    int height;
    // The JSON input to pass to the dialog when showing it.
    std::string json_input;
  };

  // When created, the property should already be set on the WebContents.
  explicit WebDialogUI(content::WebUI* web_ui);
  virtual ~WebDialogUI();

  // Close the dialog, passing the specified arguments to the close handler.
  void CloseDialog(const base::ListValue* args);

  // Returns the PropertyBag accessor object used to write the delegate pointer
  // into the WebContents (see class-level comment above).
  static base::PropertyAccessor<WebDialogDelegate*>& GetPropertyAccessor();

 private:
  // WebUIController
  virtual void RenderViewCreated(
      content::RenderViewHost* render_view_host) OVERRIDE;

  // JS message handler.
  void OnDialogClosed(const base::ListValue* args);

  DISALLOW_COPY_AND_ASSIGN(WebDialogUI);
};

// Displays external URL contents inside a modal web dialog.
//
// Intended to be the place to collect the settings and lockdowns
// necessary for running external UI components securely (e.g., the
// cloud print dialog).
class ExternalWebDialogUI : public WebDialogUI {
 public:
  explicit ExternalWebDialogUI(content::WebUI* web_ui);
  virtual ~ExternalWebDialogUI();
};

#endif  // CHROME_BROWSER_UI_WEBUI_WEB_DIALOG_UI_H_
