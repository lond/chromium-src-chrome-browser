// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/message_loop.h"
#include "chrome/browser/extensions/api/identity/web_auth_flow.h"
#include "chrome/browser/ui/extensions/web_auth_flow_window.h"
#include "chrome/test/base/chrome_render_view_host_test_harness.h"
#include "chrome/test/base/testing_profile.h"
#include "content/public/browser/browser_thread.h"
#include "content/test/test_browser_thread.h"
#include "content/test/web_contents_tester.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using content::BrowserContext;
using content::BrowserThread;
using content::TestBrowserThread;
using content::WebContents;
using content::WebContentsDelegate;
using content::WebContentsTester;
using extensions::WebAuthFlow;
using testing::Return;
using testing::ReturnRef;

namespace {

class MockDelegate : public WebAuthFlow::Delegate {
 public:
  MOCK_METHOD1(OnAuthFlowSuccess, void(const std::string& redirect_url));
  MOCK_METHOD0(OnAuthFlowFailure, void());
};

class MockWebAuthFlowWindow : public WebAuthFlowWindow {
 public:
  MockWebAuthFlowWindow() : WebAuthFlowWindow(NULL, NULL, NULL) {
  }

  virtual void Show() OVERRIDE {
    // Do nothing in tests.
  }
};

class MockWebAuthFlow : public WebAuthFlow {
 public:
  MockWebAuthFlow(
     WebAuthFlow::Delegate* delegate,
     BrowserContext* browser_context,
     const std::string& extension_id,
     const GURL& provider_url)
     : WebAuthFlow(delegate,
                   browser_context,
                   extension_id,
                   provider_url),
       browser_context_(browser_context),
       web_contents_(NULL),
       window_(NULL) { }

  virtual WebContents* CreateWebContents() OVERRIDE {
    CHECK(!web_contents_);
    web_contents_ = WebContentsTester::CreateTestWebContents(
        browser_context_, NULL);
    return web_contents_;
  }

  virtual WebAuthFlowWindow* CreateAuthWindow() OVERRIDE {
    CHECK(!window_);
    window_ = new MockWebAuthFlowWindow();
    return window_;
  }

  WebContents* contents() {
    return web_contents_;
  }

  WebContentsTester* contents_tester() {
    return WebContentsTester::For(web_contents_);
  }

  MockWebAuthFlowWindow& window() {
    return *window_;
  }

  bool HasWindow() const {
    return window_ != NULL;
  }

  virtual ~MockWebAuthFlow() { }

 private:
  BrowserContext* browser_context_;
  WebContents* web_contents_;
  MockWebAuthFlowWindow* window_;
};

}  // namespace

class WebAuthFlowTest : public ChromeRenderViewHostTestHarness {
 protected:
  WebAuthFlowTest()
      : thread_(BrowserThread::UI, &message_loop_) {
  }

  virtual void SetUp() {
    ChromeRenderViewHostTestHarness::SetUp();
  }

  void CreateAuthFlow(const std::string& extension_id, const GURL& url) {
    flow_.reset(new MockWebAuthFlow(&delegate_, profile(), extension_id, url));
  }

  MockWebAuthFlow& flow() {
    return *flow_.get();
  }

  WebAuthFlow* flow_base() {
    return flow_.get();
  }

  void CallOnClose() {
    flow_base()->OnClose();
  }

  bool CallIsValidRedirectUrl(const GURL& url) {
    return flow_base()->IsValidRedirectUrl(url);
  }

  TestBrowserThread thread_;
  MockDelegate delegate_;
  scoped_ptr<MockWebAuthFlow> flow_;
};

TEST_F(WebAuthFlowTest, SilentRedirectToChromiumAppUrl) {
  std::string ext_id = "abcdefghij";
  GURL url("https://accounts.google.com/o/oauth2/auth");
  GURL result("https://abcdefghij.chromiumapp.org/google_cb");

  CreateAuthFlow(ext_id, url);
  EXPECT_CALL(delegate_, OnAuthFlowSuccess(result.spec())).Times(1);
  flow_->Start();
  flow_->contents_tester()->NavigateAndCommit(result);
}

TEST_F(WebAuthFlowTest, SilentRedirectToChromeExtensionSchemeUrl) {
  std::string ext_id = "abcdefghij";
  GURL url("https://accounts.google.com/o/oauth2/auth");
  GURL result("chrome-extension://abcdefghij/google_cb");

  CreateAuthFlow(ext_id, url);
  EXPECT_CALL(delegate_, OnAuthFlowSuccess(result.spec())).Times(1);
  flow_->Start();
  flow_->contents_tester()->NavigateAndCommit(result);
}

TEST_F(WebAuthFlowTest, UIResultsInSuccess) {
  std::string ext_id = "abcdefghij";
  GURL url("https://accounts.google.com/o/oauth2/auth");
  GURL result("chrome-extension://abcdefghij/google_cb");

  CreateAuthFlow(ext_id, url);
  EXPECT_CALL(delegate_, OnAuthFlowSuccess(result.spec())).Times(1);
  flow_->Start();
  flow_->contents_tester()->TestSetIsLoading(false);
  EXPECT_TRUE(flow_->HasWindow());
  flow_->contents_tester()->NavigateAndCommit(result);
}

TEST_F(WebAuthFlowTest, UIClosedByUser) {
  std::string ext_id = "abcdefghij";
  GURL url("https://accounts.google.com/o/oauth2/auth");
  GURL result("chrome-extension://abcdefghij/google_cb");

  CreateAuthFlow(ext_id, url);
  EXPECT_CALL(delegate_, OnAuthFlowFailure()).Times(1);
  flow_->Start();
  flow_->contents_tester()->TestSetIsLoading(false);
  EXPECT_TRUE(flow_->HasWindow());
  CallOnClose();
}

TEST_F(WebAuthFlowTest, IsValidRedirectUrl) {
  std::string ext_id = "abcdefghij";
  GURL url("https://accounts.google.com/o/oauth2/auth");

  CreateAuthFlow(ext_id, url);

  // Positive cases.
  EXPECT_TRUE(CallIsValidRedirectUrl(
      GURL("https://abcdefghij.chromiumapp.org/")));
  EXPECT_TRUE(CallIsValidRedirectUrl(
      GURL("https://abcdefghij.chromiumapp.org/callback")));
  EXPECT_TRUE(CallIsValidRedirectUrl(
      GURL("chrome-extension://abcdefghij/")));
  EXPECT_TRUE(CallIsValidRedirectUrl(
      GURL("chrome-extension://abcdefghij/callback")));

  // Negative cases.
  EXPECT_FALSE(CallIsValidRedirectUrl(
      GURL("https://www.foo.com/")));
  // http scheme is not allowed.
  EXPECT_FALSE(CallIsValidRedirectUrl(
      GURL("http://abcdefghij.chromiumapp.org/callback")));
  EXPECT_FALSE(CallIsValidRedirectUrl(
      GURL("https://abcd.chromiumapp.org/callback")));
  EXPECT_FALSE(CallIsValidRedirectUrl(
      GURL("chrome-extension://abcd/callback")));
  EXPECT_FALSE(CallIsValidRedirectUrl(
      GURL("chrome-extension://abcdefghijkl/")));
}
