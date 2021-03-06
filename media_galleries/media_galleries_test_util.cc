// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media_galleries/media_galleries_test_util.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

#include "base/base_paths.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "chrome/browser/extensions/extension_prefs.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/extension_system.h"
#include "chrome/browser/media_galleries/fileapi/picasa_finder.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/extensions/extension.h"
#include "extensions/common/manifest_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

#if defined(OS_WIN)
#include "base/test/test_reg_util_win.h"
#include "base/win/registry.h"
#endif

scoped_refptr<extensions::Extension> AddMediaGalleriesApp(
    const std::string& name,
    const std::vector<std::string>& media_galleries_permissions,
    Profile* profile) {
  scoped_ptr<DictionaryValue> manifest(new DictionaryValue);
  manifest->SetString(extensions::manifest_keys::kName, name);
  manifest->SetString(extensions::manifest_keys::kVersion, "0.1");
  manifest->SetInteger(extensions::manifest_keys::kManifestVersion, 2);
  ListValue* background_script_list = new ListValue;
  background_script_list->Append(Value::CreateStringValue("background.js"));
  manifest->Set(extensions::manifest_keys::kPlatformAppBackgroundScripts,
                background_script_list);

  ListValue* permission_detail_list = new ListValue;
  for (size_t i = 0; i < media_galleries_permissions.size(); i++)
    permission_detail_list->Append(
        Value::CreateStringValue(media_galleries_permissions[i]));
  DictionaryValue* media_galleries_permission = new DictionaryValue();
  media_galleries_permission->Set("mediaGalleries", permission_detail_list);
  ListValue* permission_list = new ListValue;
  permission_list->Append(media_galleries_permission);
  manifest->Set(extensions::manifest_keys::kPermissions, permission_list);

  extensions::ExtensionPrefs* extension_prefs =
      extensions::ExtensionPrefs::Get(profile);
  base::FilePath path = extension_prefs->install_directory().AppendASCII(name);
  std::string errors;
  scoped_refptr<extensions::Extension> extension =
      extensions::Extension::Create(path, extensions::Manifest::INTERNAL,
                                    *manifest.get(),
                                    extensions::Extension::NO_FLAGS, &errors);
  EXPECT_TRUE(extension.get() != NULL) << errors;
  EXPECT_TRUE(extensions::Extension::IdIsValid(extension->id()));
  if (!extension.get() || !extensions::Extension::IdIsValid(extension->id()))
    return NULL;

  extension_prefs->OnExtensionInstalled(
      extension.get(),
      extensions::Extension::ENABLED,
      extensions::Blacklist::NOT_BLACKLISTED,
      syncer::StringOrdinal::CreateInitialOrdinal());
  ExtensionService* extension_service =
      extensions::ExtensionSystem::Get(profile)->extension_service();
  extension_service->AddExtension(extension.get());
  extension_service->EnableExtension(extension->id());

  return extension;
}

EnsureMediaDirectoriesExists::EnsureMediaDirectoriesExists()
    : num_galleries_(0) {
  Init();
}

EnsureMediaDirectoriesExists::~EnsureMediaDirectoriesExists() {
}

base::FilePath EnsureMediaDirectoriesExists::GetFakeAppDataPath() const {
  DCHECK(fake_dir_.IsValid());
  return fake_dir_.path().AppendASCII("appdata");
}

#if defined(OS_WIN)
base::FilePath EnsureMediaDirectoriesExists::GetFakeLocalAppDataPath() const {
  DCHECK(fake_dir_.IsValid());
  return fake_dir_.path().AppendASCII("localappdata");
}

void EnsureMediaDirectoriesExists::WriteCustomPicasaAppDataPathToRegistry(
    const base::FilePath& path) {
  base::win::RegKey key(HKEY_CURRENT_USER, picasa::kPicasaRegistryPath,
                        KEY_SET_VALUE);
  key.WriteValue(picasa::kPicasaRegistryAppDataKey, path.value().c_str());
}
#endif

#if defined(OS_WIN) || defined(OS_MACOSX)
base::FilePath
EnsureMediaDirectoriesExists::GetFakePicasaFoldersRootPath() const {
  DCHECK(fake_dir_.IsValid());
  return fake_dir_.path().AppendASCII("picasa_folders");
}
#endif

void EnsureMediaDirectoriesExists::Init() {
#if defined(OS_CHROMEOS) || defined(OS_ANDROID)
  return;
#else

  ASSERT_TRUE(fake_dir_.CreateUniqueTempDir());

#if defined(OS_WIN) || defined(OS_MACOSX)
  // This is to control whether or not tests think iTunes and Picasa are
  // installed.
  app_data_override_.reset(new base::ScopedPathOverride(
      base::DIR_APP_DATA, GetFakeAppDataPath()));
#if defined(OS_WIN)
  // Picasa on Windows is by default in the DIR_LOCAL_APP_DATA directory.
  local_app_data_override_.reset(new base::ScopedPathOverride(
      base::DIR_LOCAL_APP_DATA, GetFakeLocalAppDataPath()));
  // Picasa also looks in the registry for an alternate path.
  registry_override_.OverrideRegistry(HKEY_CURRENT_USER, L"hkcu_picasa");
#endif
#endif

  music_override_.reset(new base::ScopedPathOverride(
      chrome::DIR_USER_MUSIC, fake_dir_.path().AppendASCII("music")));
  pictures_override_.reset(new base::ScopedPathOverride(
      chrome::DIR_USER_PICTURES, fake_dir_.path().AppendASCII("pictures")));
  video_override_.reset(new base::ScopedPathOverride(
      chrome::DIR_USER_VIDEOS, fake_dir_.path().AppendASCII("videos")));
  num_galleries_ = 3;
#endif
}
