// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/download/public/common/mock_download_item_impl.h"

namespace download {

MockDownloadItemImpl::MockDownloadItemImpl(DownloadItemImplDelegate* delegate)
    : DownloadItemImpl(delegate,
                       std::string("7d122682-55b5-4a47-a253-36cadc3e5bee"),
                       DownloadItem::kInvalidId,
                       base::FilePath(),
                       base::FilePath(),
                       std::vector<GURL>(),
                       GURL(),
                       GURL(),
                       GURL(),
                       GURL(),
                       "application/octet-stream",
                       "application/octet-stream",
                       base::Time(),
                       base::Time(),
                       std::string(),
                       std::string(),
                       0,
                       0,
                       std::string(),
                       DownloadItem::COMPLETE,
                       DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS,
                       DOWNLOAD_INTERRUPT_REASON_NONE,
                       false,
                       false,
                       base::Time(),
                       true,
                       DownloadItem::ReceivedSlices()) {}

MockDownloadItemImpl::~MockDownloadItemImpl() = default;

}  // namespace download
