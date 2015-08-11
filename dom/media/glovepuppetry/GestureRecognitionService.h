/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_GestureRecognitionService_h
#define mozilla_dom_GestureRecognitionService_h

#include "nsIGestureRecognitionService.h"

class PXCSession;
class PXCSenseManager;
class PXCHandModule;
class PXCHandData;
class PXCHandConfiguration;

namespace mozilla {

PRLogModuleInfo* GetGestureRecognitionLog();
#define GR_LOG(...) MOZ_LOG(GetGestureRecognitionLog(), mozilla::LogLevel::Debug, (__VA_ARGS__))

class GestureRecognitionService final : public nsIGestureRecognitionService
{
public:
  // Add XPCOM glue code
  NS_DECL_ISUPPORTS
  NS_DECL_NSIGESTURERECOGNITIONSERVICE

  static already_AddRefed<GestureRecognitionService> FactoryCreate();

private:
  /**
   * Private destructor to prevent bypassing of reference counting
   */
  GestureRecognitionService();
  virtual ~GestureRecognitionService();

  PXCSession* mSession;
  PXCSenseManager *mSenseManager;

};

} // namespace mozilla
#endif // mozilla_dom_GestureRecognitionSwervice_h
