/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "GestureRecognitionService.h"

#include "GestureRecognition.h"
#include "mozilla/ClearOnShutdown.h"
#include "mozilla/ModuleUtils.h"

#include "Definitions.h"
#include "pxchandconfiguration.h"
#include "pxchanddata.h"
#include "pxcsensemanager.h"
#include "pxcsession.h"

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
  : mSession(nullptr)
  , mSenseManager(nullptr)
  , mHandModule(nullptr)
  , mHandDataOutput(nullptr)
  , mHandConfiguration(nullptr)
  , mStop(false)
{
  MOZ_ASSERT(NS_IsMainThread());
  MOZ_ASSERT(!gGestureService);

  // Setup
  mSession = PXCSession::CreateInstance();
  if (!mSession)
  {
    GR_LOG(("Failed Creating PXCSession"));
    return;
  }

  mSenseManager = mSession->CreateSenseManager();
  if (!mSenseManager)
  {
    ReleaseAll();
    GR_LOG(("Failed Creating PXCSenseManager"));
	  return;
  }

  if (mSenseManager->EnableHand() != PXC_STATUS_NO_ERROR)
  {
    ReleaseAll();
    GR_LOG(("Failed Enabling Hand Module"));
	  return;
  }

  mHandModule = mSenseManager->QueryHand();
  if (!mHandModule)
  {
    ReleaseAll();
    GR_LOG(("Failed Creating PXCHandModule"));
    return;
  }

  mHandDataOutput = mHandModule->CreateOutput();
  if (!mHandDataOutput)
  {
    ReleaseAll();
    GR_LOG(("Failed Creating PXCHandData"));
    return;
  }

  mHandConfiguration = mHandModule->CreateActiveConfiguration();
  if (!mHandConfiguration)
  {
    ReleaseAll();
    GR_LOG(("Failed Creating PXCHandConfiguration"));
    return;
  }

  // Enable all gestures.
  mHandConfiguration->EnableAllGestures();
  // Enable all alerts.
  mHandConfiguration->EnableAllAlerts();
  // Apply configuration setup
  mHandConfiguration->ApplyChanges();

  if (mHandConfiguration)
  {
    mHandConfiguration->Release();
    mHandConfiguration = NULL;
  }
}

GestureRecognitionService::~GestureRecognitionService()
{
	ReleaseAll();
}

void GestureRecognitionService::ReleaseAll()
{
	MOZ_ASSERT(gGestureService);
	if (mSenseManager)
	{
		mSenseManager->Close();
		mSenseManager->Release();
		mSenseManager = NULL;
	}
	if (mSession)
	{
		mSession->Release();
		mSession = NULL;
	}
}


/* void start (); */
NS_IMETHODIMP GestureRecognitionService::Start()
{
  pxcI32 numOfHands = 0;
  // First Initializing the sense manager
  if (mSenseManager->Init() == PXC_STATUS_NO_ERROR)
  {
    std::printf("\nPXCSenseManager Initializing OK\n========================\n");
    std::printf("Number of hands: %d\n", numOfHands);
    // Acquiring frames from input device
    while (mSenseManager->AcquireFrame(true) == PXC_STATUS_NO_ERROR && !mStop)
    {
      // Get current hand outputs
      if (mHandDataOutput->Update() == PXC_STATUS_NO_ERROR)
      {
        // Display alerts
        PXCHandData::AlertData alertData;
        for (int i = 0; i < mHandDataOutput->QueryFiredAlertsNumber(); ++i)
        {
          if (mHandDataOutput->QueryFiredAlertData(i, alertData) == PXC_STATUS_NO_ERROR)
          {
            std::printf("%s was fired at frame %d \n", Definitions::AlertToString(alertData.label).c_str(), alertData.frameNumber);
          }
        }

        // Display gestures
        PXCHandData::GestureData gestureData;
        for (int i = 0; i < mHandDataOutput->QueryFiredGesturesNumber(); ++i)
        {
          if (mHandDataOutput->QueryFiredGestureData(i, gestureData) == PXC_STATUS_NO_ERROR)
          {
            std::wprintf(L"%s, Gesture: %s was fired at frame %d \n", Definitions::GestureStateToString(gestureData.state), gestureData.name, gestureData.frameNumber);
          }
        }

        // Display joints
        PXCHandData::IHand *hand;
        PXCHandData::JointData jointData;
        for (int i = 0; i < mHandDataOutput->QueryNumberOfHands(); ++i)
        {
          mHandDataOutput->QueryHandData(PXCHandData::ACCESS_ORDER_BY_TIME, i, hand);
          std::string handSide = "Unknown Hand";
          handSide = hand->QueryBodySide() == PXCHandData::BODY_SIDE_LEFT ? "Left Hand" : "Right Hand";

          std::printf("%s\n==============\n", handSide.c_str());
          for (int j = 0; j < 22; ++j)
          {
            if (hand->QueryTrackedJoint((PXCHandData::JointType)j, jointData) == PXC_STATUS_NO_ERROR)
            {
              std::printf("     %s)\tX: %f, Y: %f, Z: %f \n", Definitions::JointToString((PXCHandData::JointType)j).c_str(), jointData.positionWorld.x, jointData.positionWorld.y, jointData.positionWorld.z);
            }
          }
        }

        // Display number of hands
        if (numOfHands != mHandDataOutput->QueryNumberOfHands())
        {
          numOfHands = mHandDataOutput->QueryNumberOfHands();
          std::printf("Number of hands: %d\n", numOfHands);
        }

      } // end if update

      mSenseManager->ReleaseFrame();
    } // end while acquire frame
  } else {
    ReleaseAll();
    std::printf("Failed Initializing PXCSenseManager\n");
    return NS_ERROR_NOT_INITIALIZED;
  }
  return NS_OK;
}

/* void stop (); */
NS_IMETHODIMP GestureRecognitionService::Stop()
{
  ReleaseAll();
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
