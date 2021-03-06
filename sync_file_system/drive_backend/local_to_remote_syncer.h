// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SYNC_FILE_SYSTEM_DRIVE_BACKEND_LOCAL_TO_REMOTE_SYNCER_H_
#define CHROME_BROWSER_SYNC_FILE_SYSTEM_DRIVE_BACKEND_LOCAL_TO_REMOTE_SYNCER_H_

#include "chrome/browser/sync_file_system/sync_callbacks.h"
#include "chrome/browser/sync_file_system/sync_task.h"

namespace sync_file_system {
namespace drive_backend {

class LocalToRemoteSyncer : public SyncTask {
 public:
  LocalToRemoteSyncer();
  virtual ~LocalToRemoteSyncer();
  virtual void Run(const SyncStatusCallback& callback) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(LocalToRemoteSyncer);
};

}  // namespace drive_backend
}  // namespace sync_file_system

#endif  // CHROME_BROWSER_SYNC_FILE_SYSTEM_DRIVE_BACKEND_LOCAL_TO_REMOTE_SYNCER_H_
