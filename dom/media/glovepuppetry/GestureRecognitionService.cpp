/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "GestureRecognitionService.h"

#include "GestureRecognition.h"
#include "mozilla/ClearOnShutdown.h"
#include "mozilla/ModuleUtils.h"
#include "nsGlobalWindow.h"
#include "nsIController.h"
#include "nsPIWindowRoot.h"

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
volatile bool g_stop = true;

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
}

GestureRecognitionService::~GestureRecognitionService()
{
	ReleaseAll();
}
void GestureRecognitionService::ReleaseAll()
{
  if (mSession)
	{
		mSession->Release();
		mSession = NULL;
	}
}
class DispatchHandler : public PXCSenseManager::Handler {
public:
  virtual pxcStatus PXCAPI OnModuleProcessedFrame(pxcUID mid, PXCBase
                                                  *module, PXCCapture::Sample *sample) {
    // check if the callback is from the hand tracking module
    if (mid == PXCHandModule::CUID) {
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
        if (mNumOfHands != mHandDataOutput->QueryNumberOfHands())
        {
          mNumOfHands = mHandDataOutput->QueryNumberOfHands();
          std::printf("Number of hands: %d\n", mNumOfHands);
        }
      }
    }
    // return NO_ERROR to continue, or any error to abort
    return PXC_STATUS_NO_ERROR;
  }

  DispatchHandler(PXCHandData *data)
    :mHandDataOutput(data)
    ,mNumOfHands(0)
  {
  }

protected:
  PXCHandData *mHandDataOutput;
  pxcI32 mNumOfHands;
};

class GestureRecognitionService::LaunchGestureRecognitionRunnable final : public nsRunnable
{
public:
  LaunchGestureRecognitionRunnable(PXCSession *aSession)
    : mSession(aSession)
    , mSenseManager(nullptr)
    , mScriptGlobal(nullptr)
  {
    mScriptGlobal = 
    NS_NewScriptGlobalObject(false, false);
    MOZ_ASSERT(mSession);
  }

  ~LaunchGestureRecognitionRunnable()
  {
    ReleaseAll();
  }

  void ReleaseAll()
  {
    MOZ_ASSERT(gGestureService);
    if (mSenseManager)
    {
      mSenseManager->Close();
      mSenseManager->Release();
      mSenseManager = NULL;
    }
  }

  NS_IMETHOD Run() override
  {
    mSenseManager = mSession->CreateSenseManager();
    if (!mSenseManager)
    {
      ReleaseAll();
      GR_LOG(("Failed Creating PXCSenseManager"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    if (mSenseManager->EnableHand() != PXC_STATUS_NO_ERROR)
    {
      ReleaseAll();
      GR_LOG(("Failed Enabling Hand Module"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    PXCHandModule* handModule = mSenseManager->QueryHand();
    if (!handModule)
    {
      ReleaseAll();
      GR_LOG(("Failed Creating PXCHandModule"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    PXCHandData* handDataOutput = handModule->CreateOutput();
    if (!handDataOutput)
    {
      ReleaseAll();
      GR_LOG(("Failed Creating PXCHandData"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    PXCHandConfiguration* handConfiguration = handModule->CreateActiveConfiguration();
    if (!handConfiguration)
    {
      ReleaseAll();
      GR_LOG(("Failed Creating PXCHandConfiguration"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    // Enable all gestures.
//    handConfiguration->EnableAllGestures();
    handConfiguration->DisableAllGestures();
    handConfiguration->EnableGesture(L"thumb_up", true);
    handConfiguration->EnableGesture(L"thumb_down", true);
    handConfiguration->EnableGesture(L"swipe_up", true);
    handConfiguration->EnableGesture(L"swipe_down", true);
    handConfiguration->EnableGesture(L"swipe_left", true);
    handConfiguration->EnableGesture(L"swipe_right", true);
    handConfiguration->ApplyChanges();

    // Enable all alerts.
//    handConfiguration->EnableAllAlerts();
    handConfiguration->DisableAllAlerts();
    // Apply configuration setup
    handConfiguration->ApplyChanges();
    handConfiguration->Update();

    if (handConfiguration)
    {
      handConfiguration->Release();
      handConfiguration = NULL;
    }
    pxcI32 numOfHands = 0;
    bool bLastGestureIsThumbUp = false;
    bool bLastGestureIsThumbDown = false;
    if (mSenseManager->Init() == PXC_STATUS_NO_ERROR)
    {
      // Acquiring frames from input device
      while (mSenseManager->AcquireFrame(true) == PXC_STATUS_NO_ERROR && !g_stop)
      {
        // Get current hand outputs
        if (handDataOutput->Update() == PXC_STATUS_NO_ERROR)
        {
          // Display alerts
          PXCHandData::AlertData alertData;
          for (int i = 0; i < handDataOutput->QueryFiredAlertsNumber(); ++i)
          {
            if (handDataOutput->QueryFiredAlertData(i, alertData) == PXC_STATUS_NO_ERROR)
            {
              std::printf("%s was fired at frame %d \n", Definitions::AlertToString(alertData.label).c_str(), alertData.frameNumber);
            }
          }

          // Display gestures
          PXCHandData::GestureData gestureData;
          for (int i = 0; i < handDataOutput->QueryFiredGesturesNumber(); ++i)
          {
            if (handDataOutput->QueryFiredGestureData(i, gestureData) == PXC_STATUS_NO_ERROR)
            {
              std::wprintf(L"%s, Gesture: %s was fired at frame %d \n", Definitions::GestureStateToString(gestureData.state), gestureData.name, gestureData.frameNumber);
            }
          }
          if (handDataOutput->IsGestureFired(L"thumb_up", gestureData)) {
            // handle tap gesture
            if (!bLastGestureIsThumbUp) {
              ErrorResult er;
              mScriptGlobal->BackOuter(er);
              NS_WARN_IF(er.Failed());
              bLastGestureIsThumbUp = true;
              bLastGestureIsThumbDown = false;

            }
          }
          if (handDataOutput->IsGestureFired(L"thumb_down", gestureData)) {
            // handle tap gesture
            if (!bLastGestureIsThumbDown) {
              ErrorResult er;
              mScriptGlobal->ForwardOuter(er);
              NS_WARN_IF(er.Failed());
              bLastGestureIsThumbDown = true;
              bLastGestureIsThumbUp = false;
            }
          }

          /*
          // Display joints
          PXCHandData::IHand *hand;
          PXCHandData::JointData jointData;
          for (int i = 0; i < handDataOutput->QueryNumberOfHands(); ++i)
          {
            handDataOutput->QueryHandData(PXCHandData::ACCESS_ORDER_BY_TIME, i, hand);
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
          */
          // Display number of hands
          if (numOfHands != handDataOutput->QueryNumberOfHands())
          {
            numOfHands = handDataOutput->QueryNumberOfHands();
            std::printf("Number of hands: %d\n", numOfHands);
          }
        }
      mSenseManager->ReleaseFrame();
      } // end while acquire frame
      handDataOutput->Release();
    } // end if Init
    else
    {
      ReleaseAll();
      std::printf("Failed Initializing PXCSenseManager\n");
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }
    return NS_OK;
  }

private:
  PXCSession *mSession;
  PXCSenseManager* mSenseManager;
  nsRefPtr<nsGlobalWindow> mScriptGlobal;
};


/* void start (); */
NS_IMETHODIMP GestureRecognitionService::Start()
{
    std::printf("\nPXCSenseManager Initializing OK\n========================\n");

    nsresult rv = NS_NewNamedThread("GRThread", getter_AddRefs(mThread));
    if (NS_FAILED(rv)) {
      NS_WARNING("Can't create gesture recognition worker thread.");
      return rv;
    }
    g_stop = false;
    rv = mThread->Dispatch(new LaunchGestureRecognitionRunnable(mSession),
                           nsIEventTarget::DISPATCH_NORMAL);
    if (NS_FAILED(rv)) {
      return rv;
    }
  return NS_OK;
}

/* void stop (); */
NS_IMETHODIMP GestureRecognitionService::Stop()
{
  std::printf("\n GestureRecognitionService::Stop() \n========================\n");
  std::printf("\n========================\n");
  std::printf("\n========================\n");
  std::printf("\n========================\n");
  g_stop = true;
  MOZ_ASSERT(mThread);

  mThread->Shutdown();
  mThread = nullptr; // deletes GR worker thread

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
