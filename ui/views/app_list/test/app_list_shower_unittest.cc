// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_path.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/app_list/keep_alive_service.h"
#include "chrome/browser/ui/app_list/test/fake_keep_alive_service.h"
#include "chrome/browser/ui/app_list/test/fake_profile.h"
#include "chrome/browser/ui/views/app_list/app_list_shower.h"
#include "chrome/browser/ui/views/app_list/app_list_view_factory.h"
#include "chrome/browser/ui/views/app_list/app_list_view_win.h"
#include "testing/gtest/include/gtest/gtest.h"

class FakeView : public AppListViewWin {
 public:
  explicit FakeView(Profile* profile)
      : profile_(profile) {
  }

  std::string profile_name() {
    return profile_->GetProfileName();
  }

  // AppListViewWin overrides.
  virtual void Show() OVERRIDE {
    visible_ = true;
  }

  virtual void Hide() OVERRIDE {
    visible_ = false;
  }

  virtual void MoveNearCursor() OVERRIDE {
  }

  virtual bool IsVisible() OVERRIDE {
    return visible_;
  }

  virtual void Prerender() OVERRIDE {
    prerendered_ = true;
  }

  virtual void RegainNextLostFocus() OVERRIDE {
  }

  virtual gfx::NativeWindow GetWindow() OVERRIDE {
    return NULL;
  }

  virtual void SetProfile(Profile* profile) OVERRIDE {
    profile_ = profile;
  }

  Profile* profile_;
  bool visible_;
  bool prerendered_;
};

class FakeFactory : public AppListViewFactory {
 public:
  FakeFactory()
      : views_created_(0) {
  }

  virtual AppListViewWin* CreateAppListView(
      Profile* profile,
      const base::Closure& on_should_dismiss) OVERRIDE {
    views_created_++;
    return new FakeView(profile);
  }

  int views_created_;
};

class AppListShowerUnitTest : public testing::Test {
 public:
  virtual void SetUp() OVERRIDE {
    keep_alive_service_ = new FakeKeepAliveService;
    factory_ = new FakeFactory;
    shower_.reset(new AppListShower(make_scoped_ptr(factory_),
                                    make_scoped_ptr(keep_alive_service_)));
    profile1_ = CreateProfile("p1").Pass();
    profile2_ = CreateProfile("p2").Pass();
  }

  virtual void TearDown() OVERRIDE {
  }

  scoped_ptr<FakeProfile> CreateProfile(const std::string& name) {
    return make_scoped_ptr(new FakeProfile(name));
  }

  FakeView* GetCurrentView() {
    return static_cast<FakeView*>(shower_->view());
  }

  // Owned by |shower_|.
  FakeKeepAliveService* keep_alive_service_;
  // Owned by |shower_|.
  FakeFactory* factory_;
  scoped_ptr<AppListShower> shower_;
  scoped_ptr<FakeProfile> profile1_;
  scoped_ptr<FakeProfile> profile2_;
};

TEST_F(AppListShowerUnitTest, Preconditions) {
  EXPECT_FALSE(shower_->IsAppListVisible());
  EXPECT_FALSE(shower_->HasView());
  EXPECT_FALSE(keep_alive_service_->is_keeping_alive());
}

TEST_F(AppListShowerUnitTest, ShowForProfilePutsViewOnScreen) {
  shower_->ShowForProfile(profile1_.get());
  EXPECT_TRUE(shower_->IsAppListVisible());
  EXPECT_TRUE(shower_->HasView());
  EXPECT_TRUE(keep_alive_service_->is_keeping_alive());
}

TEST_F(AppListShowerUnitTest, HidingViewRemovesKeepalive) {
  shower_->ShowForProfile(profile1_.get());
  shower_->DismissAppList();
  EXPECT_FALSE(shower_->IsAppListVisible());
  EXPECT_TRUE(shower_->HasView());
  EXPECT_FALSE(keep_alive_service_->is_keeping_alive());
}

TEST_F(AppListShowerUnitTest, HideAndShowReusesView) {
  shower_->ShowForProfile(profile1_.get());
  shower_->DismissAppList();
  shower_->ShowForProfile(profile1_.get());
  EXPECT_EQ(1, factory_->views_created_);
}

TEST_F(AppListShowerUnitTest, CloseAndShowRecreatesView) {
  shower_->ShowForProfile(profile1_.get());
  shower_->CloseAppList();
  shower_->ShowForProfile(profile1_.get());
  EXPECT_EQ(2, factory_->views_created_);
}

TEST_F(AppListShowerUnitTest, CloseRemovesView) {
  shower_->ShowForProfile(profile1_.get());
  shower_->CloseAppList();
  EXPECT_FALSE(shower_->IsAppListVisible());
  EXPECT_FALSE(shower_->HasView());
  EXPECT_FALSE(keep_alive_service_->is_keeping_alive());
}

TEST_F(AppListShowerUnitTest, SwitchingProfiles) {
  shower_->ShowForProfile(profile1_.get());
  EXPECT_EQ("p1", GetCurrentView()->profile_name());
  shower_->ShowForProfile(profile2_.get());
  EXPECT_EQ("p2", GetCurrentView()->profile_name());

  // Shouldn't create new view for second profile - it should switch in place.
  EXPECT_EQ(1, factory_->views_created_);
}
