/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "GestureRecognitionService.h"

#include "GestureRecognition.h"
#include "mozilla/ClearOnShutdown.h"
#include "mozilla/ModuleUtils.h"
//#include "pxcsession.h"

#define NS_GESTURESERVICE_CID \
  { 0xc8942aa4, 0xe421, 0x483f, \
    { 0xac, 0xfc, 0x67, 0xe5, 0x10, 0x62, 0x60, 0xc7 } }
#define NS_GESTURESERVICE_CONTRACTID "@mozilla.org/glovepuppetry/gestureservice;1"

namespace mozilla {

static GestureRecognitionService* gGestureService;

NS_IMPL_ISUPPORTS(GestureRecognitionService, nsIGestureRecognitionService)

already_AddRefed<GestureRecognitionService>
GestureRecognitionService::FactoryCreate()
{
  MOZ_ASSERT(NS_IsMainThread());

  if (!gGestureService) {
    gGestureService = new GestureRecognitionService();
    ClearOnShutdown(&gGestureService);
  }

  nsRefPtr<GestureRecognitionService> service = gGestureService;
  return service.forget();
}

GestureRecognitionService::GestureRecognitionService()
{
  MOZ_ASSERT(NS_IsMainThread());
  MOZ_ASSERT(!gGestureService);
  GR_LOG(("Test\n"));

  // Setup
#if 0
  mSession = PXCSession::CreateInstance();
  if (!mSession)
  {
    GR_LOG(("Failed Creating PXCSession\n"));
    return;
  }
#endif

}

GestureRecognitionService::~GestureRecognitionService()
{
  MOZ_ASSERT(!gGestureService);
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


NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(GestureRecognitionService,
                                         GestureRecognitionService::FactoryCreate)

NS_DEFINE_NAMED_CID(NS_GESTURESERVICE_CID);

static const mozilla::Module::CIDEntry kGestureServiceCIDs[] = {
  { &kNS_GESTURESERVICE_CID, false, nullptr, GestureRecognitionServiceConstructor },
  { nullptr }
};

static const mozilla::Module::ContractIDEntry kGestureServiceContracts[] = {
  { NS_GESTURESERVICE_CONTRACTID, &kNS_GESTURESERVICE_CID },
  { nullptr }
};

static const mozilla::Module kGestureServiceModule = {
  mozilla::Module::kVersion,
  kGestureServiceCIDs,
  kGestureServiceContracts,
  nullptr
};

} // namespace mozilla

NSMODULE_DEFN(GestureServiceModule) = &mozilla::kGestureServiceModule;
