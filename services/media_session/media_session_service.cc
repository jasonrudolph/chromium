// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/media_session/media_session_service.h"

#include "base/bind.h"
#include "services/media_session/audio_focus_manager.h"

namespace media_session {

MediaSessionService::MediaSessionService(
    service_manager::mojom::ServiceRequest request)
    : service_binding_(this, std::move(request)),
      audio_focus_manager_(std::make_unique<AudioFocusManager>()) {
  registry_.AddInterface(
      base::BindRepeating(&AudioFocusManager::BindToInterface,
                          base::Unretained(audio_focus_manager_.get())));
  registry_.AddInterface(
      base::BindRepeating(&AudioFocusManager::BindToDebugInterface,
                          base::Unretained(audio_focus_manager_.get())));
  registry_.AddInterface(
      base::BindRepeating(&AudioFocusManager::BindToActiveControllerInterface,
                          base::Unretained(audio_focus_manager_.get())));
}

MediaSessionService::~MediaSessionService() = default;

void MediaSessionService::OnBindInterface(
    const service_manager::BindSourceInfo& source_info,
    const std::string& interface_name,
    mojo::ScopedMessagePipeHandle interface_pipe) {
  registry_.BindInterface(interface_name, std::move(interface_pipe));
}

}  // namespace media_session
