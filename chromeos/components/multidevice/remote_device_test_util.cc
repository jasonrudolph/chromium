// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <string>

#include "chromeos/components/multidevice/remote_device_test_util.h"

#include "base/base64.h"

namespace chromeos {

namespace multidevice {

// Attributes of the default test remote device.
const char kTestRemoteDeviceUserId[] = "example@gmail.com";
const char kTestRemoteDeviceName[] = "remote device";
const char kTestRemoteDevicePublicKey[] = "public key";
const char kTestRemoteDevicePSK[] = "remote device psk";
const int64_t kTestRemoteDeviceLastUpdateTimeMillis = 0L;

RemoteDeviceRefBuilder::RemoteDeviceRefBuilder() {
  remote_device_ = std::make_shared<RemoteDevice>(CreateRemoteDeviceForTest());
}

RemoteDeviceRefBuilder::~RemoteDeviceRefBuilder() = default;

RemoteDeviceRefBuilder& RemoteDeviceRefBuilder::SetUserId(
    const std::string& user_id) {
  remote_device_->user_id = user_id;
  return *this;
}

RemoteDeviceRefBuilder& RemoteDeviceRefBuilder::SetName(
    const std::string& name) {
  remote_device_->name = name;
  return *this;
}

RemoteDeviceRefBuilder& RemoteDeviceRefBuilder::SetPublicKey(
    const std::string& public_key) {
  remote_device_->public_key = public_key;
  return *this;
}

RemoteDeviceRefBuilder& RemoteDeviceRefBuilder::SetSupportsMobileHotspot(
    bool supports_mobile_hotspot) {
  remote_device_
      ->software_features[cryptauth::SoftwareFeature::MAGIC_TETHER_HOST] =
      supports_mobile_hotspot ? SoftwareFeatureState::kSupported
                              : SoftwareFeatureState::kNotSupported;
  return *this;
}

RemoteDeviceRefBuilder& RemoteDeviceRefBuilder::SetSoftwareFeatureState(
    const cryptauth::SoftwareFeature feature,
    const SoftwareFeatureState new_state) {
  remote_device_->software_features[feature] = new_state;
  return *this;
}

RemoteDeviceRefBuilder& RemoteDeviceRefBuilder::SetLastUpdateTimeMillis(
    int64_t last_update_time_millis) {
  remote_device_->last_update_time_millis = last_update_time_millis;
  return *this;
}

RemoteDeviceRefBuilder& RemoteDeviceRefBuilder::SetBeaconSeeds(
    const std::vector<cryptauth::BeaconSeed>& beacon_seeds) {
  remote_device_->beacon_seeds = beacon_seeds;
  return *this;
}

RemoteDeviceRef RemoteDeviceRefBuilder::Build() {
  return RemoteDeviceRef(remote_device_);
}

RemoteDevice CreateRemoteDeviceForTest() {
  std::map<cryptauth::SoftwareFeature, SoftwareFeatureState> software_features;
  software_features[cryptauth::SoftwareFeature::EASY_UNLOCK_HOST] =
      SoftwareFeatureState::kEnabled;
  software_features[cryptauth::SoftwareFeature::MAGIC_TETHER_HOST] =
      SoftwareFeatureState::kSupported;

  return RemoteDevice(kTestRemoteDeviceUserId, kTestRemoteDeviceName,
                      kTestRemoteDevicePublicKey, kTestRemoteDevicePSK,
                      kTestRemoteDeviceLastUpdateTimeMillis, software_features,
                      {} /* beacon_seeds */);
}

RemoteDeviceRef CreateRemoteDeviceRefForTest() {
  return RemoteDeviceRefBuilder().Build();
}

RemoteDeviceRefList CreateRemoteDeviceRefListForTest(size_t num_to_create) {
  RemoteDeviceRefList generated_devices;

  for (size_t i = 0; i < num_to_create; i++) {
    RemoteDeviceRef remote_device =
        RemoteDeviceRefBuilder()
            .SetPublicKey("publicKey" + std::to_string(i))
            .Build();
    generated_devices.push_back(remote_device);
  }

  return generated_devices;
}

RemoteDeviceList CreateRemoteDeviceListForTest(size_t num_to_create) {
  RemoteDeviceList generated_devices;

  for (size_t i = 0; i < num_to_create; i++) {
    RemoteDevice remote_device = CreateRemoteDeviceForTest();
    remote_device.public_key = "publicKey" + std::to_string(i);
    generated_devices.push_back(remote_device);
  }

  return generated_devices;
}

RemoteDevice* GetMutableRemoteDevice(const RemoteDeviceRef& remote_device_ref) {
  const RemoteDevice* remote_device = remote_device_ref.remote_device_.get();
  return const_cast<RemoteDevice*>(remote_device);
}

bool IsSameDevice(const RemoteDevice& remote_device,
                  RemoteDeviceRef remote_device_ref) {
  if (!remote_device_ref.remote_device_)
    return false;

  return remote_device == *remote_device_ref.remote_device_;
}

}  // namespace multidevice

}  // namespace chromeos
