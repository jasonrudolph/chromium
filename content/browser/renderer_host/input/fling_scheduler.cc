// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/renderer_host/input/fling_scheduler.h"

#include "build/build_config.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "ui/compositor/compositor.h"

#if defined(USE_AURA)
#include "ui/aura/window.h"
#include "ui/aura/window_tree_host.h"
#endif

namespace content {

FlingScheduler::FlingScheduler(RenderWidgetHostImpl* host) : host_(host) {
  DCHECK(host);
}

FlingScheduler::~FlingScheduler() {
  if (observed_compositor_)
    observed_compositor_->RemoveAnimationObserver(this);
}

void FlingScheduler::ScheduleFlingProgress() {
  // Don't do anything if a ui::Compositor is already being observed.
  if (observed_compositor_)
    return;

  host_->SetNeedsBeginFrameForFlingProgress();
}

void FlingScheduler::RegisterFlingSchedulerObserver(
    base::WeakPtr<FlingController> fling_controller) {
  DCHECK(fling_controller);
  fling_controller_ = fling_controller;
  // Convert the following if-statement back to DCHECK(!observed_compositor_)
  // once https://crbug.com/897216 is fixed.
  if (observed_compositor_)
    return;

  ui::Compositor* compositor = GetCompositor();
  // If a ui::Compositor can't be obtained, ask the host for BeginFrames.
  if (!compositor)
    return;

  compositor->AddAnimationObserver(this);
  observed_compositor_ = compositor;
}

void FlingScheduler::UnregisterFlingSchedulerObserver() {
  if (observed_compositor_) {
    observed_compositor_->RemoveAnimationObserver(this);
    observed_compositor_ = nullptr;
  }
  fling_controller_ = nullptr;
}

void FlingScheduler::DidStopFlingingOnBrowser() {
  host_->DidStopFlinging();
}

bool FlingScheduler::NeedsBeginFrameForFlingProgress() {
  return !GetCompositor();
}

void FlingScheduler::ProgressFlingOnBeginFrameIfneeded(
    base::TimeTicks current_time) {
  // No fling is active.
  if (!fling_controller_)
    return;

  // FlingProgress will be called within FlingController::OnAnimationStep.
  if (observed_compositor_)
    return;

  fling_controller_->ProgressFling(current_time);
}

ui::Compositor* FlingScheduler::GetCompositor() {
#if defined(USE_AURA)
  if (host_->GetView() && host_->GetView()->GetNativeView() &&
      host_->GetView()->GetNativeView()->GetHost() &&
      host_->GetView()->GetNativeView()->GetHost()->compositor()) {
    return host_->GetView()->GetNativeView()->GetHost()->compositor();
  }
#endif

  return nullptr;
}

void FlingScheduler::OnAnimationStep(base::TimeTicks timestamp) {
  DCHECK(observed_compositor_);
  if (fling_controller_)
    fling_controller_->ProgressFling(timestamp);
}

void FlingScheduler::OnCompositingShuttingDown(ui::Compositor* compositor) {
  DCHECK_EQ(observed_compositor_, compositor);
  observed_compositor_->RemoveAnimationObserver(this);
  observed_compositor_ = nullptr;
}

}  // namespace content
