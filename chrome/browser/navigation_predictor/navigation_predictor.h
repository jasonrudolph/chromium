// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_NAVIGATION_PREDICTOR_NAVIGATION_PREDICTOR_H_
#define CHROME_BROWSER_NAVIGATION_PREDICTOR_NAVIGATION_PREDICTOR_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "base/optional.h"
#include "base/sequence_checker.h"
#include "base/time/time.h"
#include "content/public/browser/visibility.h"
#include "content/public/browser/web_contents_observer.h"
#include "mojo/public/cpp/bindings/interface_request.h"
#include "third_party/blink/public/mojom/loader/navigation_predictor.mojom.h"
#include "url/origin.h"

#ifdef OS_ANDROID
#include "base/android/application_status_listener.h"
#endif  // OS_ANDROID

namespace content {
class BrowserContext;
class RenderFrameHost;
}

class SiteEngagementService;
class TemplateURLService;

// This class gathers metrics of anchor elements from both renderer process
// and browser process. Then it uses these metrics to make predictions on what
// are the most likely anchor elements that the user will click.
class NavigationPredictor : public blink::mojom::AnchorElementMetricsHost,
                            public content::WebContentsObserver {
 public:
  // |render_frame_host| is the host associated with the render frame. It is
  // used to retrieve metrics at the browser side.
  explicit NavigationPredictor(content::RenderFrameHost* render_frame_host);
  ~NavigationPredictor() override;

  // Create and bind NavigationPredictor.
  static void Create(mojo::InterfaceRequest<AnchorElementMetricsHost> request,
                     content::RenderFrameHost* render_frame_host);

  // Enum describing the possible set of actions that navigation predictor may
  // take. This enum should remain synchronized with enum
  // NavigationPredictorActionTaken in enums.xml. Order of enum values should
  // not be changed since the values are recorded in UMA.
  enum class Action {
    kUnknown = 0,
    kNone = 1,
    kPreresolve = 2,
    kPreconnect = 3,
    kPrefetch = 4,
    kPreconnectOnVisibilityChange = 5,
    kPreconnectOnAppForeground = 6,
    kMaxValue = kPreconnectOnAppForeground,
  };

  // Enum describing the accuracy of actions taken by the navigation predictor.
  // This enum should remain synchronized with enum
  // NavigationPredictorAccuracyActionTaken in enums.xml. Order of enum values
  // should not be changed since the values are recorded in UMA.
  enum class ActionAccuracy {
    // No action was taken, but an anchor element was clicked.
    kNoActionTakenClickHappened = 0,

    // Navigation predictor prefetched a URL, and an anchor element was clicked
    // which pointed to the same URL as prefetched URL.
    kPrefetchActionClickToSameURL = 1,

    // Navigation predictor prefetched a URL, and an anchor element was clicked
    // whose URL had the same origin as the prefetched URL.
    kPrefetchActionClickToSameOrigin = 2,

    // Navigation predictor prefetched a URL, and an anchor element was clicked
    // whose URL had a different origin than the prefetched URL.
    kPrefetchActionClickToDifferentOrigin = 3,

    // Navigation predictor preconnected to an origin, and an anchor element was
    // clicked whose URL had the same origin as the preconnected origin.
    kPreconnectActionClickToSameOrigin = 4,

    // Navigation predictor preconnected to an origin, and an anchor element was
    // clicked whose URL had a different origin than the preconnected origin.
    kPreconnectActionClickToDifferentOrigin = 5,
    kMaxValue = kPreconnectActionClickToDifferentOrigin,
  };

 protected:
  // Origin that we decided to preconnect to.
  base::Optional<url::Origin> preconnect_origin_;

  // URL that we decided to prefetch.
  base::Optional<GURL> prefetch_url_;

 private:
  // Struct holding navigation score, rank and other info of the anchor element.
  // Used for look up when an anchor element is clicked.
  struct NavigationScore;

  // blink::mojom::AnchorElementMetricsHost:
  void ReportAnchorElementMetricsOnClick(
      blink::mojom::AnchorElementMetricsPtr metrics) override;
  void ReportAnchorElementMetricsOnLoad(
      std::vector<blink::mojom::AnchorElementMetricsPtr> metrics) override;

  // Returns true if the anchor element metric from the renderer process is
  // valid.
  bool IsValidMetricFromRenderer(
      const blink::mojom::AnchorElementMetrics& metric) const;

  // Returns site engagement service, which can be used to get site engagement
  // score. Return value is guaranteed to be non-null.
  SiteEngagementService* GetEngagementService() const;

  // Returns template URL service. Guaranteed to be non-null.
  TemplateURLService* GetTemplateURLService() const;

  // Merge anchor element metrics that have the same target url (href).
  void MergeMetricsSameTargetUrl(
      std::vector<blink::mojom::AnchorElementMetricsPtr>* metrics) const;

  // Computes and stores document level metrics, including |number_of_anchors_|,
  // |document_engagement_score_|, etc.
  void ComputeDocumentMetricsOnLoad(
      const std::vector<blink::mojom::AnchorElementMetricsPtr>& metrics);

  // Given metrics of an anchor element from both renderer and browser process,
  // returns navigation score. Virtual for testing purposes.
  virtual double CalculateAnchorNavigationScore(
      const blink::mojom::AnchorElementMetrics& metrics,
      double document_engagement_score,
      double target_engagement_score,
      int area_rank,
      int number_of_anchors) const;

  // Given a vector of navigation scores sorted in descending order, decide what
  // action to take, or decide not to do anything. Example actions including
  // preresolve, preload, prerendering, etc.
  void MaybeTakeActionOnLoad(
      const url::Origin& document_origin,
      const std::vector<std::unique_ptr<NavigationScore>>&
          sorted_navigation_scores);

  base::Optional<GURL> GetUrlToPrefetch(
      const url::Origin& document_origin,
      const std::vector<std::unique_ptr<NavigationScore>>&
          sorted_navigation_scores) const;

  base::Optional<url::Origin> GetOriginToPreconnect(
      const url::Origin& document_origin,
      const std::vector<std::unique_ptr<NavigationScore>>&
          sorted_navigation_scores) const;

  // Record anchor element metrics on page load.
  void RecordMetricsOnLoad(
      const blink::mojom::AnchorElementMetrics& metric) const;

  // Record timing information when an anchor element is clicked.
  void RecordTimingOnClick();

  // Records the accuracy of the action taken by the navigator predictor based
  // on the action taken as well as the URL that was navigated to.
  // |target_url| is the URL navigated to by the user.
  void RecordActionAccuracyOnClick(const GURL& target_url) const;

  // content::WebContentsObserver:
  void OnVisibilityChanged(content::Visibility visibility) override;

#ifdef OS_ANDROID
  // Called when application state changes. e.g., application is brought to the
  // background or the foreground.
  void OnApplicationStateChange(
      base::android::ApplicationState application_state);
#endif  // OS_ANDROID

  // Called when tab or app visibility change. Takes a pre* action.
  // |log_action| should be set to the reason why this method was called.
  void TakeActionNowOnTabOrAppVisibilityChange(Action log_action);

  // Used to get keyed services.
  content::BrowserContext* const browser_context_;

  // Maps from target url (href) to navigation score.
  std::unordered_map<std::string, std::unique_ptr<NavigationScore>>
      navigation_scores_map_;

  // Total number of anchors that: href has the same host as the document,
  // contains image, inside an iframe, href incremented by 1 from document url.
  int number_of_anchors_same_host_ = 0;
  int number_of_anchors_contains_image_ = 0;
  int number_of_anchors_in_iframe_ = 0;
  int number_of_anchors_url_incremented_ = 0;

  // Scaling factors used to compute navigation scores.
  const int ratio_area_scale_;
  const int is_in_iframe_scale_;
  const int is_same_host_scale_;
  const int contains_image_scale_;
  const int is_url_incremented_scale_;
  const int source_engagement_score_scale_;
  const int target_engagement_score_scale_;
  const int area_rank_scale_;

  // Sum of all scales. Used to normalize the final computed weight.
  const int sum_scales_;

  // True if device is a low end device.
  const bool is_low_end_device_;

  // Minimum score that a URL should have for it to be prefetched. Note
  // that scores of origins are computed differently from scores of URLs, so
  // they are not comparable.
  const int prefetch_url_score_threshold_;

  // Minimum preconnect score that the origin should have for preconnect. Note
  // that scores of origins are computed differently from scores of URLs, so
  // they are not comparable.
  const int preconnect_origin_score_threshold_;

  // Timing of document loaded and last click.
  base::TimeTicks document_loaded_timing_;
  base::TimeTicks last_click_timing_;

  // True if the source webpage (i.e., the page on which we are trying to
  // predict the next navigation) is a page from user's default search engine.
  bool source_is_default_search_engine_page_ = false;

  // Current visibility state of the web contents.
  content::Visibility current_visibility_;

#ifdef OS_ANDROID
  // Used to listen to the changes in the application state changes. e.g., when
  // the application is brought to the background or the foreground.
  std::unique_ptr<base::android::ApplicationStatusListener>
      application_status_listener_;

  base::android::ApplicationState application_state_;
#endif  // OS_ANDROID

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(NavigationPredictor);
};

#endif  // CHROME_BROWSER_NAVIGATION_PREDICTOR_NAVIGATION_PREDICTOR_H_
