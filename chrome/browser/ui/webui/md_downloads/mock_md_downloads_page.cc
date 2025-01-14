// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/md_downloads/mock_md_downloads_page.h"

MockPage::MockPage() : binding_(this) {}
MockPage::~MockPage() = default;

md_downloads::mojom::PagePtr MockPage::BindAndGetPtr() {
  DCHECK(!binding_.is_bound());
  md_downloads::mojom::PagePtr page;
  binding_.Bind(mojo::MakeRequest(&page));
  return page;
}
