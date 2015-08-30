/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_TesseractTextRecognitionService_h
#define mozilla_TesseractTextRecognitionService_h

#include "nsITextRecognitionService.h"
#include "nsIThread.h"
#include "nsTArray.h"

namespace mozilla {
using namespace dom;

class TesseractTextRecognitionService final : public nsITextRecognitionService
{
public:
  // Add XPCOM glue code
  NS_DECL_ISUPPORTS
  NS_DECL_NSITEXTRECOGNITIONSERVICE

  static already_AddRefed<TesseractTextRecognitionService> FactoryCreate();

private:
  class DoingTextRecognitionRunnable;

  /**
   * Private destructor to prevent bypassing of reference counting
   */
  TesseractTextRecognitionService();
  virtual ~TesseractTextRecognitionService() {}

  nsCOMPtr<nsIThread> mThread;
};

} // namespace mozilla
#endif // mozilla_TesseractTextRecognitionService_h
