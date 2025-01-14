// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_SERVICE_MANAGER_SANDBOX_FUCHSIA_SANDBOX_POLICY_FUCHSIA_H_
#define SERVICES_SERVICE_MANAGER_SANDBOX_FUCHSIA_SANDBOX_POLICY_FUCHSIA_H_

#include <lib/zx/channel.h>

#include "base/memory/ref_counted.h"
#include "services/service_manager/sandbox/export.h"
#include "services/service_manager/sandbox/sandbox_type.h"

namespace base {
struct LaunchOptions;
class SequencedTaskRunner;

namespace fuchsia {
class FilteredServiceDirectory;
}  // namespace fuchsia

}  // namespace base

namespace service_manager {

class SERVICE_MANAGER_SANDBOX_EXPORT SandboxPolicyFuchsia {
 public:
  SandboxPolicyFuchsia();
  ~SandboxPolicyFuchsia();

  // Initializes the policy of the given sandbox |type|. Must be called on the
  // IO thread.
  void Initialize(service_manager::SandboxType type);

  // Modifies the process launch |options| to achieve  the level of
  // isolation appropriate for current the sandbox type. The caller may then add
  // any descriptors or handles afterward to grant additional capabilities
  // to the new process.
  void UpdateLaunchOptionsForSandbox(base::LaunchOptions* options);

 private:
  service_manager::SandboxType type_ = service_manager::SANDBOX_TYPE_INVALID;

  // Services directory used for the /svc namespace of the child process.
  std::unique_ptr<base::fuchsia::FilteredServiceDirectory> service_directory_;
  zx::channel service_directory_client_channel_;
  scoped_refptr<base::SequencedTaskRunner> service_directory_task_runner_;

  DISALLOW_COPY_AND_ASSIGN(SandboxPolicyFuchsia);
};

}  // namespace service_manager

#endif  // SERVICES_SERVICE_MANAGER_SANDBOX_FUCHSIA_SANDBOX_POLICY_FUCHSIA_H_
