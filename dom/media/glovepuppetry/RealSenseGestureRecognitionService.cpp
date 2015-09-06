/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "RealSenseGestureRecognitionService.h"

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
#include "pxctouchlesscontroller.h"

#define NS_REALSENSE_GESTURESERVICE_CID \
{ 0x5a9704f3, 0xa685, 0x4e68, \
    { 0x82, 0x55, 0x6a, 0x36, 0xba, 0x45, 0xa1, 0xaa } }
#define NS_REALSENSE_GESTURESERVICE_CONTRACTID NS_GESTURE_RECOGNITION_SERVICE_CONTRACTID_PREFIX "real-sense"

namespace mozilla {

using namespace dom;

static RealSenseGestureRecognitionService* gGestureService;
volatile bool g_stop = true;

NS_IMPL_ISUPPORTS(RealSenseGestureRecognitionService, nsIGestureRecognitionService)

already_AddRefed<RealSenseGestureRecognitionService>
RealSenseGestureRecognitionService::FactoryCreate()
{
  MOZ_ASSERT(NS_IsMainThread());

  if (!gGestureService) {
    gGestureService = new RealSenseGestureRecognitionService();
    ClearOnShutdown(&gGestureService);
  }

  nsRefPtr<RealSenseGestureRecognitionService> service = gGestureService;
  return service.forget();
}

RealSenseGestureRecognitionService::RealSenseGestureRecognitionService()
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

RealSenseGestureRecognitionService::~RealSenseGestureRecognitionService()
{
	ReleaseAll();
}
void RealSenseGestureRecognitionService::ReleaseAll()
{
  if (mSession)
	{
		mSession->Release();
		mSession = NULL;
	}
}



class RealSenseGestureRecognitionService::LaunchGestureRecognitionRunnable final : public nsRunnable
{
public:
  LaunchGestureRecognitionRunnable(PXCSession *aSession, nsTArray<WeakPtr<GestureRecognition>>* aGestureRecognitions)
    : mSession(aSession)
    , mSenseManager(nullptr)
    , mGestureRecognitions(aGestureRecognitions)
  {
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
      std::printf(("Failed Creating PXCSenseManager"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    if (mSenseManager->EnableHand() != PXC_STATUS_NO_ERROR)
    {
      ReleaseAll();
      std::printf(("Failed Enabling Hand Module"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    PXCHandModule* handModule = mSenseManager->QueryHand();
    if (!handModule)
    {
      ReleaseAll();
      std::printf(("Failed Creating PXCHandModule"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    PXCHandData* handDataOutput = handModule->CreateOutput();
    if (!handDataOutput)
    {
      ReleaseAll();
      std::printf(("Failed Creating PXCHandData"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    PXCHandConfiguration* handConfiguration = handModule->CreateActiveConfiguration();
    if (!handConfiguration)
    {
      ReleaseAll();
      std::printf(("Failed Creating PXCHandConfiguration"));
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
            if (!bLastGestureIsThumbUp) {
              bLastGestureIsThumbUp = true;
              bLastGestureIsThumbDown = false;
              for (size_t i = 0; i < mGestureRecognitions->Length(); ++i) {
                WeakPtr<GestureRecognition> recognition = (*mGestureRecognitions)[i];
                if (recognition) {
                  recognition->NotifyThumbUp();
                }
              }
            }
          }
          if (handDataOutput->IsGestureFired(L"thumb_down", gestureData)) {
            if (!bLastGestureIsThumbDown) {
              bLastGestureIsThumbDown = true;
              bLastGestureIsThumbUp = false;
              for (size_t i = 0; i < mGestureRecognitions->Length(); ++i) {
                WeakPtr<GestureRecognition> recognition = (*mGestureRecognitions)[i];
                if (recognition) {
                  recognition->NotifyThumbDown();
                }
              }
            }
          }
          if (handDataOutput->IsGestureFired(L"swipe_up", gestureData)) {
            // Clear thumb gesture when swipe is triggered.
            bLastGestureIsThumbUp = false;
            bLastGestureIsThumbDown = false;
            for (size_t i = 0; i < mGestureRecognitions->Length(); ++i) {
              WeakPtr<GestureRecognition> recognition = (*mGestureRecognitions)[i];
              if (recognition) {
                recognition->NotifySwipeUp();
              }
            }
          }
          if (handDataOutput->IsGestureFired(L"swipe_down", gestureData)) {
            // Clear thumb gesture when swipe is triggered.
            bLastGestureIsThumbUp = false;
            bLastGestureIsThumbDown = false;
            for (size_t i = 0; i < mGestureRecognitions->Length(); ++i) {
              WeakPtr<GestureRecognition> recognition = (*mGestureRecognitions)[i];
              if (recognition) {
                recognition->NotifySwipeDown();
              }
            }
          }
          if (handDataOutput->IsGestureFired(L"swipe_left", gestureData)) {
            // Clear thumb gesture when swipe is triggered.
            bLastGestureIsThumbUp = false;
            bLastGestureIsThumbDown = false;
            for (size_t i = 0; i < mGestureRecognitions->Length(); ++i) {
              WeakPtr<GestureRecognition> recognition = (*mGestureRecognitions)[i];
              if (recognition) {
                recognition->NotifySwipeLeft();
              }
            }
          }
          if (handDataOutput->IsGestureFired(L"swipe_right", gestureData)) {
            // Clear thumb gesture when swipe is triggered.
            bLastGestureIsThumbUp = false;
            bLastGestureIsThumbDown = false;
            for (size_t i = 0; i < mGestureRecognitions->Length(); ++i) {
              WeakPtr<GestureRecognition> recognition = (*mGestureRecognitions)[i];
              if (recognition) {
                recognition->NotifySwipeRight();
              }
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
  nsTArray<WeakPtr<GestureRecognition>>* mGestureRecognitions;
};


class RealSenseGestureRecognitionService::TouchlessGestureRecognitionRunnable final : public nsRunnable
{
public:
  TouchlessGestureRecognitionRunnable(PXCSession *aSession, nsTArray<WeakPtr<GestureRecognition>>* aGestureRecognitions)
    : mSession(aSession)
    , mSenseManager(nullptr)
    , mGestureRecognitions(aGestureRecognitions)
  {
    MOZ_ASSERT(mSession);
  }

  ~TouchlessGestureRecognitionRunnable()
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

  class EventHandler : public PXCTouchlessController::UXEventHandler
  {
  public:
    EventHandler(nsTArray<WeakPtr<GestureRecognition>>* aGestureRecognitions)
      : mGestureRecognitions(aGestureRecognitions)
    {
    }

    virtual void PXCAPI OnFiredUXEvent(const PXCTouchlessController::UXEventData *uxEventData) {
      switch (uxEventData->type) {
      case PXCTouchlessController::UXEventData::UXEvent_StartScroll:
        std::printf("@@ UXEvent_StartScroll\n");
        break;
      case PXCTouchlessController::UXEventData::UXEvent_ScrollUp:
        std::printf("@@ UXEvent_ScrollUp\n");
        for (size_t i = 0; i < mGestureRecognitions->Length(); ++i) {
          WeakPtr<GestureRecognition> recognition = (*mGestureRecognitions)[i];
          if (recognition) {
            recognition->NotifySwipeUp();
          }
        }
        break;
      case PXCTouchlessController::UXEventData::UXEvent_ScrollDown:
        std::printf("@@ UXEvent_ScrollDown\n");
        for (size_t i = 0; i < mGestureRecognitions->Length(); ++i) {
          WeakPtr<GestureRecognition> recognition = (*mGestureRecognitions)[i];
          if (recognition) {
            recognition->NotifySwipeDown();
          }
        }
        break;
      case PXCTouchlessController::UXEventData::UXEvent_ScrollLeft:
        std::printf("@@ UXEvent_ScrollLeft\n");
        for (size_t i = 0; i < mGestureRecognitions->Length(); ++i) {
          WeakPtr<GestureRecognition> recognition = (*mGestureRecognitions)[i];
          if (recognition) {
            recognition->NotifySwipeLeft();
          }
        }
        break;
      case PXCTouchlessController::UXEventData::UXEvent_ScrollRight:
        std::printf("@@ UXEvent_ScrollRight\n");
        for (size_t i = 0; i < mGestureRecognitions->Length(); ++i) {
          WeakPtr<GestureRecognition> recognition = (*mGestureRecognitions)[i];
          if (recognition) {
            recognition->NotifySwipeRight();
          }
        }
        break;
      case PXCTouchlessController::UXEventData::UXEvent_EndScroll:
        std::printf("@@ UXEvent_EndScroll\n");
        break;
        // handle all events
      }
    }
  private:
    nsTArray<WeakPtr<GestureRecognition>>* mGestureRecognitions;
  };

  NS_IMETHOD Run() override
  {
    mSenseManager = mSession->CreateSenseManager();
    if (!mSenseManager)
    {
      ReleaseAll();
      std::printf(("\n@@@Failed Creating PXCSenseManager\n@@@"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }
#if 0
    if (mSenseManager->EnableHand() != PXC_STATUS_NO_ERROR)
    {
      ReleaseAll();
      std::printf(("Failed Enabling Hand Module"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    PXCHandModule* handModule = mSenseManager->QueryHand();
    if (!handModule)
    {
      ReleaseAll();
      std::printf(("Failed Creating PXCHandModule"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    PXCHandData* handDataOutput = handModule->CreateOutput();
    if (!handDataOutput)
    {
      ReleaseAll();
      std::printf(("Failed Creating PXCHandData"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    PXCHandConfiguration* handConfiguration = handModule->CreateActiveConfiguration();
    if (!handConfiguration)
    {
      ReleaseAll();
      std::printf(("Failed Creating PXCHandConfiguration"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    // Enable some gestures.
    handConfiguration->DisableAllGestures();
    handConfiguration->EnableGesture(L"thumb_up", true);
    handConfiguration->EnableGesture(L"thumb_down", true);
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
#endif
    pxcI32 numOfHands = 0;
    bool bLastGestureIsThumbUp = false;
    bool bLastGestureIsThumbDown = false;
    
    // Touchless controller
    if (mSenseManager->EnableTouchlessController() != PXC_STATUS_NO_ERROR)
    {
      ReleaseAll();
      std::printf(("\n@@@Failed Enabling Touchless Contorller Module\n@@@"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    PXCTouchlessController * touchlessModule = mSenseManager->QueryTouchlessController();
    if (!touchlessModule)
    {
      ReleaseAll();
      std::printf(("\n@@@Failed Creating PXCTouchlessController\n@@@"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    // register for ux events
    EventHandler eventHandler(mGestureRecognitions);
    if (touchlessModule->SubscribeEvent(&eventHandler) != PXC_STATUS_NO_ERROR)
    {
      ReleaseAll();
      std::printf(("\n@@@Failed SubscribeEvent\n@@@"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    // Configure Touchless Controller
    PXCTouchlessController::ProfileInfo profInfo;
    touchlessModule->QueryProfile(&profInfo);
    profInfo.config |= (PXCTouchlessController::ProfileInfo::Configuration_Scroll_Horizontally
                        | PXCTouchlessController::ProfileInfo::Configuration_Scroll_Vertically
                        //| PXCTouchlessController::ProfileInfo::Configuration_Enable_Injection
                        );

    if (touchlessModule->SetProfile(&profInfo) != PXC_STATUS_NO_ERROR)
    {
      ReleaseAll();
      std::printf(("\n@@@Failed SetProfile\n@@@"));
      return NS_ERROR_DOM_NOT_FOUND_ERR;
    }

    if (mSenseManager->Init() == PXC_STATUS_NO_ERROR)
    {
      // Acquiring frames from input device
      while (mSenseManager->AcquireFrame(true) == PXC_STATUS_NO_ERROR && !g_stop)
      {
        // Get current hand outputs
#if 0
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
            if (!bLastGestureIsThumbUp) {
              bLastGestureIsThumbUp = true;
              bLastGestureIsThumbDown = false;
              for (size_t i = 0; i < mGestureRecognitions->Length(); ++i) {
                WeakPtr<GestureRecognition> recognition = (*mGestureRecognitions)[i];
                if (recognition) {
                  recognition->NotifyThumbUp();
                }
              }
            }
          }
          if (handDataOutput->IsGestureFired(L"thumb_down", gestureData)) {
            if (!bLastGestureIsThumbDown) {
              bLastGestureIsThumbDown = true;
              bLastGestureIsThumbUp = false;
              for (size_t i = 0; i < mGestureRecognitions->Length(); ++i) {
                WeakPtr<GestureRecognition> recognition = (*mGestureRecognitions)[i];
                if (recognition) {
                  recognition->NotifyThumbDown();
                }
              }
            }
          }

          // Display number of hands
          if (numOfHands != handDataOutput->QueryNumberOfHands())
          {
            numOfHands = handDataOutput->QueryNumberOfHands();
            std::printf("Number of hands: %d\n", numOfHands);
          }
        }
#endif
        mSenseManager->ReleaseFrame();
      } // end while acquire frame
//      handDataOutput->Release();
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
  nsTArray<WeakPtr<GestureRecognition>>* mGestureRecognitions;
};


/* void start (); */
NS_IMETHODIMP RealSenseGestureRecognitionService::Start(WeakPtr<GestureRecognition> aGestureRecognition)
{
  std::printf("\nmGestureListenter.Length() = %d\n", mGestureRecognitions.Length());
  if (mGestureRecognitions.IsEmpty()) {
    std::printf("\nPXCSenseManager Initializing OK\n========================\n");
    std::printf("\n========================\n");
    std::printf("\n========================\n");
    nsresult rv = NS_NewNamedThread("GRThread", getter_AddRefs(mThread));
    if (NS_FAILED(rv)) {
      NS_WARNING("Can't create gesture recognition worker thread.");
      return rv;
    }
    g_stop = false;
#if 1   
    rv = mThread->Dispatch(new LaunchGestureRecognitionRunnable(mSession, &mGestureRecognitions),
                           nsIEventTarget::DISPATCH_NORMAL);
#else
    rv = mThread->Dispatch(new TouchlessGestureRecognitionRunnable(mSession, &mGestureRecognitions),
                           nsIEventTarget::DISPATCH_NORMAL);
#endif
    if (NS_FAILED(rv)) {
      return rv;
    }
  }
  mGestureRecognitions.AppendElement(aGestureRecognition);
  std::printf("\nmGestureListenter.Length() = %d\n", mGestureRecognitions.Length());
  return NS_OK;
}

/* void stop (); */
NS_IMETHODIMP RealSenseGestureRecognitionService::Stop(WeakPtr<GestureRecognition> aGestureRecognition)
{
  std::printf("\nmGestureListenter.Length() = %d\n", mGestureRecognitions.Length());

  mGestureRecognitions.RemoveElement(aGestureRecognition);
  if (mGestureRecognitions.IsEmpty()) {
    std::printf("\n RealSenseGestureRecognitionService::Stop() \n========================\n");
    std::printf("\n========================\n");
    std::printf("\n========================\n");

    g_stop = true;
    MOZ_ASSERT(mThread);

    mThread->Shutdown();
    mThread = nullptr; // deletes GR worker thread
  }
  return NS_OK;
}


NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(RealSenseGestureRecognitionService,
                                         RealSenseGestureRecognitionService::FactoryCreate)

NS_DEFINE_NAMED_CID(NS_REALSENSE_GESTURESERVICE_CID);

static const mozilla::Module::CIDEntry kRealSenseGestureServiceCIDs[] = {
  { &kNS_REALSENSE_GESTURESERVICE_CID, false, nullptr, RealSenseGestureRecognitionServiceConstructor },
  { nullptr }
};

static const mozilla::Module::ContractIDEntry kRealSenseGestureServiceContracts[] = {
  { NS_REALSENSE_GESTURESERVICE_CONTRACTID, &kNS_REALSENSE_GESTURESERVICE_CID },
  { nullptr }
};

static const mozilla::Module kGestureServiceModule = {
  mozilla::Module::kVersion,
  kRealSenseGestureServiceCIDs,
  kRealSenseGestureServiceContracts,
  nullptr
};

} // namespace mozilla

NSMODULE_DEFN(GestureServiceModule) = &mozilla::kGestureServiceModule;
