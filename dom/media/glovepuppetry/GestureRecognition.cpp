/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "GestureRecognition.h"

#include "nsIGestureRecognitionService.h"
#include "mozilla/dom/GestureRecognitionBinding.h"
#include "mozilla/Preferences.h"
#include "nsCycleCollectionParticipant.h"
#include "nsServiceManagerUtils.h"

namespace mozilla {

PRLogModuleInfo*
GetGestureRecognitionLog()
{
  static PRLogModuleInfo* sLog;
  if (!sLog) {
    sLog = PR_NewLogModule("GestureRecognition");
  }

  return sLog;
}

namespace dom {

#define PREFERENCE_DEFAULT_GESTURE_RECOGNITION_SERVICE "media.gesture.recognition.service.default"
#define DEFAULT_GESTURE_RECOGNITION_SERVICE "real-sense"

already_AddRefed<nsIGestureRecognitionService>
GetGestureRecognitionService()
{
  nsAutoCString gestureRecognitionServiceCID;

  nsAdoptingCString prefValue =
    Preferences::GetCString(PREFERENCE_DEFAULT_GESTURE_RECOGNITION_SERVICE);
  nsAutoCString gestureRecognitionService;

  if (!prefValue.IsEmpty()) {
    gestureRecognitionService = prefValue;
  }
  else {
    gestureRecognitionService = DEFAULT_GESTURE_RECOGNITION_SERVICE;
  }

  gestureRecognitionServiceCID =
    NS_LITERAL_CSTRING(NS_GESTURE_RECOGNITION_SERVICE_CONTRACTID_PREFIX) +
      gestureRecognitionService;

  nsresult rv;
  nsCOMPtr<nsIGestureRecognitionService> recognitionService;
  recognitionService = do_GetService(gestureRecognitionServiceCID.get(), &rv);
  return recognitionService.forget();
}
NS_IMPL_CYCLE_COLLECTION_INHERITED(GestureRecognition, DOMEventTargetHelper, mRecognitionService)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION_INHERITED(GestureRecognition)
NS_INTERFACE_MAP_END_INHERITING(DOMEventTargetHelper)

NS_IMPL_ADDREF_INHERITED(GestureRecognition, DOMEventTargetHelper)
NS_IMPL_RELEASE_INHERITED(GestureRecognition, DOMEventTargetHelper)

GestureRecognition::GestureRecognition(nsPIDOMWindow* aOwnerWindow, ErrorResult& aRv)
  : DOMEventTargetHelper(aOwnerWindow)
  , mRecognitionService(nullptr)
{
  GR_LOG("created GestureRecognition");
  if (!SetRecognitionService(aRv)) {
    return;
  }
}

JSObject*
GestureRecognition::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return GestureRecognitionBinding::Wrap(aCx, this, aGivenProto);
}

nsISupports*
GestureRecognition::GetParentObject() const
{
  return GetOwner();
}

// static
already_AddRefed<GestureRecognition>
GestureRecognition::Constructor(const GlobalObject& aGlobal, ErrorResult& aRv)
{
  nsCOMPtr<nsPIDOMWindow> win = do_QueryInterface(aGlobal.GetAsSupports());
  if (!win) {
    aRv.Throw(NS_ERROR_FAILURE);
  }

  MOZ_ASSERT(win->IsInnerWindow());
  nsRefPtr<GestureRecognition> object = new GestureRecognition(win, aRv);
  return object.forget();
}

bool
GestureRecognition::SetRecognitionService(ErrorResult& aRv)
{
  mRecognitionService = GetGestureRecognitionService();

  if (!mRecognitionService) {
    aRv.Throw(NS_ERROR_DOM_INVALID_STATE_ERR);
    return false;
  }
  return true;
}

class GestureEventRunnable : public nsRunnable
{
public: 
  GestureEventRunnable(GestureRecognition* aGesture, const nsAString& aEventType)
    : mGesture(aGesture)
    , mEventType(aEventType)
  {
  }

  NS_IMETHOD Run() override
  {
    nsresult rv;
    rv = mGesture->DispatchTrustedEvent(mEventType);
    NS_WARN_IF(NS_FAILED(rv));
    return NS_OK;
  }
  GestureRecognition* mGesture;
  nsAutoString mEventType;
};

void GestureRecognition::NotifyThumbUp()
{
  nsRefPtr<GestureEventRunnable> runnable = new GestureEventRunnable(this, NS_LITERAL_STRING("thumbup"));
  NS_DispatchToMainThread(runnable);
}

void GestureRecognition::NotifyThumbDown()
{
  nsRefPtr<GestureEventRunnable> runnable = new GestureEventRunnable(this, NS_LITERAL_STRING("thumbdown"));
  NS_DispatchToMainThread(runnable);
}

void GestureRecognition::NotifySwipeUp()
{
  nsRefPtr<GestureEventRunnable> runnable = new GestureEventRunnable(this, NS_LITERAL_STRING("swipeup"));
  NS_DispatchToMainThread(runnable);
}

void GestureRecognition::NotifySwipeDown()
{
  nsRefPtr<GestureEventRunnable> runnable = new GestureEventRunnable(this, NS_LITERAL_STRING("swipedown"));
  NS_DispatchToMainThread(runnable);
}

void GestureRecognition::NotifySwipeLeft()
{
  nsRefPtr<GestureEventRunnable> runnable = new GestureEventRunnable(this, NS_LITERAL_STRING("swipeleft"));
  NS_DispatchToMainThread(runnable);
}

void GestureRecognition::NotifySwipeRight()
{
  nsRefPtr<GestureEventRunnable> runnable = new GestureEventRunnable(this, NS_LITERAL_STRING("swiperight"));
  NS_DispatchToMainThread(runnable);
}

void GestureRecognition::Start(ErrorResult& aRv)
{
  nsresult rv;
  // TODO: Should pass media stream as argument in the future.
  rv = mRecognitionService->Start(this);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return;
  }
}

void GestureRecognition::Stop()
{
  nsresult rv;
  rv = mRecognitionService->Stop(this);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return;
  }
}

} // namespace dom
} // namespace mozilla
