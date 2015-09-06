/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_RealSenseGestureRecognitionService_h
#define mozilla_dom_RealSenseGestureRecognitionService_h

#include "nsIGestureRecognitionService.h"

class PXCSession;
class PXCSenseManager;
class PXCHandModule;
class PXCHandData;
class PXCHandConfiguration;

namespace mozilla {
using namespace dom;

class RealSenseGestureRecognitionService final : public nsIGestureRecognitionService
{
public:
  // Add XPCOM glue code
  NS_DECL_ISUPPORTS
  NS_DECL_NSIGESTURERECOGNITIONSERVICE

  static already_AddRefed<RealSenseGestureRecognitionService> FactoryCreate();

private:
  class TouchlessGestureRecognitionRunnable;
  class LaunchGestureRecognitionRunnable;

  /**
   * Private destructor to prevent bypassing of reference counting
   */
  RealSenseGestureRecognitionService();
  virtual ~RealSenseGestureRecognitionService();
  void ReleaseAll();

  PXCSession* mSession;

  nsCOMPtr<nsIThread> mThread;
  /** The associated GestureRecognition */
  nsTArray<WeakPtr<GestureRecognition>> mGestureRecognitions;
};

} // namespace mozilla
#endif // mozilla_dom_RealSenseGestureRecognitionService_h
