// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/services/multidevice_setup/host_status_provider.h"

#include "base/logging.h"
#include "chromeos/components/proximity_auth/logging/logging.h"

namespace chromeos {

namespace multidevice_setup {

HostStatusProvider::HostStatusWithDevice::HostStatusWithDevice(
    mojom::HostStatus host_status,
    const base::Optional<multidevice::RemoteDeviceRef>& host_device)
    : host_status_(host_status), host_device_(host_device) {
  if (host_status_ == mojom::HostStatus::kNoEligibleHosts ||
      host_status_ == mojom::HostStatus::kEligibleHostExistsButNoHostSet) {
    if (host_device_) {
      PA_LOG(ERROR) << "HostStatusWithDevice::HostStatusWithDevice(): Tried to "
                    << "construct a HostStatusWithDevice with a status "
                    << "indicating no device, but a device was provided. "
                    << "Status: " << host_status_ << ", Device ID: "
                    << host_device_->GetTruncatedDeviceIdForLogs();
      NOTREACHED();
    }
  } else if (!host_device_) {
    PA_LOG(ERROR) << "HostStatusWithDevice::HostStatusWithDevice(): Tried to "
                  << "construct a HostStatusWithDevice with a status "
                  << "indicating a device, but no device was provided. "
                  << "Status: " << host_status_;
    NOTREACHED();
  }
}

HostStatusProvider::HostStatusWithDevice::HostStatusWithDevice(
    const HostStatusWithDevice& other) = default;

HostStatusProvider::HostStatusWithDevice::~HostStatusWithDevice() = default;

bool HostStatusProvider::HostStatusWithDevice::operator==(
    const HostStatusWithDevice& other) const {
  return host_status_ == other.host_status_ &&
         host_device_ == other.host_device_;
}

bool HostStatusProvider::HostStatusWithDevice::operator!=(
    const HostStatusWithDevice& other) const {
  return !(*this == other);
}

HostStatusProvider::HostStatusProvider() = default;

HostStatusProvider::~HostStatusProvider() = default;

void HostStatusProvider::AddObserver(Observer* observer) {
  observer_list_.AddObserver(observer);
}

void HostStatusProvider::RemoveObserver(Observer* observer) {
  observer_list_.RemoveObserver(observer);
}

void HostStatusProvider::NotifyHostStatusChange(
    mojom::HostStatus host_status,
    const base::Optional<multidevice::RemoteDeviceRef>& host_device) {
  HostStatusWithDevice host_status_with_device(host_status, host_device);
  for (auto& observer : observer_list_)
    observer.OnHostStatusChange(host_status_with_device);
}

}  // namespace multidevice_setup

}  // namespace chromeos
