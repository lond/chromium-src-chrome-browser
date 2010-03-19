// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/login/issue_response_handler.h"

#include <string>

#include "chrome/browser/net/chrome_url_request_context.h"
#include "chrome/browser/net/url_fetcher.h"
#include "net/base/load_flags.h"

const int kMaxRedirs = 2;
const int kTimeout = 2;

// Overridden from AuthResponseHandler.
bool IssueResponseHandler::CanHandle(const GURL& url) {
  return (url.spec().find(AuthResponseHandler::kIssueAuthTokenUrl) !=
          std::string::npos);
}

// Overridden from AuthResponseHandler.
URLFetcher* IssueResponseHandler::Handle(
    const std::string& to_process,
    URLFetcher::Delegate* catcher) {
  LOG(INFO) << "IssueAuthToken successful!";
  token_url_.assign(StringPrintf("%s%s",
                                 AuthResponseHandler::kTokenAuthUrl,
                                 to_process.c_str()));
  URLFetcher* fetcher =
      new URLFetcher(GURL(token_url_), URLFetcher::GET, catcher);
  fetcher->set_load_flags(net::LOAD_DO_NOT_SEND_COOKIES);
  if (getter_) {
    fetcher->set_request_context(getter_);
    fetcher->Start();
  }
  return fetcher;
}
