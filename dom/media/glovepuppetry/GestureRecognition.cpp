/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "GestureRecognition.h"

#include "mozilla/dom/GestureRecognitionBinding.h"
#include "pxcsession.h"

already_AddRefed<nsIGestureRecognitionService>
NS_CreateGestureRecognitionService()
{
  nsCOMPtr<nsIGestureRecognitionService> service;

  service = new mozilla::GestureRecognitionService();

  return service.forget();
}

namespace mozilla {

NS_IMPL_ISUPPORTS(GestureRecognitionService, nsIGestureRecognitionService)

GestureRecognitionService::GestureRecognitionService()
{
  /* member initializers and constructor code */
}

GestureRecognitionService::~GestureRecognitionService()
{
  /* destructor code */
}

/* void initialize (); */
NS_IMETHODIMP GestureRecognitionService::Initialize()
{
  return NS_OK;
}

/* void start (); */
NS_IMETHODIMP GestureRecognitionService::Start()
{
  return NS_OK;
}

/* void stop (); */
NS_IMETHODIMP GestureRecognitionService::Stop()
{
  return NS_OK;
}

namespace dom {

PRLogModuleInfo*
GetGestureRecognitionLog()
{
  static PRLogModuleInfo* sLog;
  if (!sLog) {
    sLog = PR_NewLogModule("GestureRecognition");
  }

  return sLog;
}

GestureRecognition::GestureRecognition() {
  // Setup
  mSession = PXCSession::CreateInstance();
  if (!mSession)
  {
    GR_LOG(("Failed Creating PXCSession\n"));
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

void GestureRecognition::Start()
{

}

void GestureRecognition::Stop()
{

}


} // namespace dom
} // namespace mozilla