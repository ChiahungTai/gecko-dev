/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_GestureRecognition_h
#define mozilla_dom_GestureRecognition_h

#include "mozilla/Logging.h"

namespace mozilla {

namespace dom {

PRLogModuleInfo* GetGestureRecognitionLog();
#define GR_LOG(...) MOZ_LOG(GetGestureRecognitionLog(), mozilla::LogLevel::Debug, (__VA_ARGS__))

class GestureRecognition final
{
public:
  explicit GestureRecognition();

};

} // namespace dom
} // namespace mozilla
#endif // mozilla_dom_GestureRecognition_h