/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_GestureRecognition_h
#define mozilla_dom_GestureRecognition_h

#include "mozilla/DOMEventTargetHelper.h"
#include "mozilla/Logging.h"
#include "mozilla/WeakPtr.h"
#include "nsIGestureRecognitionService.h"

namespace mozilla {

extern PRLogModuleInfo* GetGestureRecognitionLog();
#define GR_LOG(...) MOZ_LOG(GetGestureRecognitionLog(), mozilla::LogLevel::Debug, (__VA_ARGS__))

namespace dom {

class GestureRecognition final : public DOMEventTargetHelper
                               , public SupportsWeakPtr<GestureRecognition>
{
public:
  MOZ_DECLARE_WEAKREFERENCE_TYPENAME(GestureRecognition)
  explicit GestureRecognition(nsPIDOMWindow* aOwnerWindow, ErrorResult& aRv);

  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(GestureRecognition, DOMEventTargetHelper)

  nsISupports* GetParentObject() const;

  virtual JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  virtual void NotifyThumbUp();
  virtual void NotifyThumbDown();
  virtual void NotifySwipeUp();
  virtual void NotifySwipeDown();
  virtual void NotifySwipeLeft();
  virtual void NotifySwipeRight();

  // WebIDL:
  static already_AddRefed<GestureRecognition>
  Constructor(const GlobalObject& aGlobal, ErrorResult& aRv);
  void Start(ErrorResult& aRv);
  void Stop();

  IMPL_EVENT_HANDLER(thumbup)
  IMPL_EVENT_HANDLER(thumbdown)
  IMPL_EVENT_HANDLER(swipeup)
  IMPL_EVENT_HANDLER(swipedown)
  IMPL_EVENT_HANDLER(swipeleft)
  IMPL_EVENT_HANDLER(swiperight)

private:
  ~GestureRecognition(){};
  bool SetRecognitionService(ErrorResult& aRv);

  nsCOMPtr<nsIGestureRecognitionService> mRecognitionService;
};

} // namespace dom
} // namespace mozilla
#endif // mozilla_dom_GestureRecognition_h
