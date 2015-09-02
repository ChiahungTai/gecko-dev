/*
 * TextRecognition.cpp
 *
 *  Created on: 2015年8月27日
 *      Author: ctai
 */

#include "TextRecognition.h"

#include "nsCycleCollectionParticipant.h"
#include "mozilla/dom/TextRecognitionBinding.h"
#include "mozilla/dom/ImageBitmap.h"
#include "mozilla/dom/TextRecognitizedEvent.h"

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_CLASS(TextRecognition)                                       \
NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN_INHERITED(TextRecognition, DOMEventTargetHelper)               \
NS_IMPL_CYCLE_COLLECTION_UNLINK_END                                          \
NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN_INHERITED(TextRecognition, DOMEventTargetHelper)             \
NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION_INHERITED(TextRecognition)
NS_INTERFACE_MAP_END_INHERITING(DOMEventTargetHelper)

NS_IMPL_ADDREF_INHERITED(TextRecognition, DOMEventTargetHelper)
NS_IMPL_RELEASE_INHERITED(TextRecognition, DOMEventTargetHelper)

#define DEFAULT_TEXT_RECOGNITION_SERVICE "tesseract"

already_AddRefed<nsITextRecognitionService>
GetTextRecognitionService()
{
  nsAutoCString textRecognitionServiceCID;

//  nsAdoptingCString prefValue =
//    Preferences::GetCString(PREFERENCE_DEFAULT_TEXT_RECOGNITION_SERVICE);
  nsAutoCString textRecognitionService;

//  if (!prefValue.IsEmpty()) {
//    gestureRecognitionService = prefValue;
//  }
//  else {
  textRecognitionService = DEFAULT_TEXT_RECOGNITION_SERVICE;
//  }

  textRecognitionServiceCID =
    NS_LITERAL_CSTRING(NS_TEXT_RECOGNITION_SERVICE_CONTRACTID_PREFIX) +
    textRecognitionService;

  nsresult rv;
  nsCOMPtr<nsITextRecognitionService> recognitionService;
  recognitionService = do_GetService(textRecognitionServiceCID.get(), &rv);
  return recognitionService.forget();
}

TextRecognition::TextRecognition(nsPIDOMWindow* aOwnerWindow)
  : DOMEventTargetHelper(aOwnerWindow)
{
  ErrorResult rv;
  if (!SetRecognitionService(rv)) {
    return;
  }
}

JSObject*
TextRecognition::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return TextRecognitionBinding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<TextRecognition>
TextRecognition::Constructor(const GlobalObject& aGlobal,
                               ErrorResult& aRv)
{
  nsCOMPtr<nsPIDOMWindow> win = do_QueryInterface(aGlobal.GetAsSupports());
  if (!win) {
    aRv.Throw(NS_ERROR_FAILURE);
  }

  MOZ_ASSERT(win->IsInnerWindow());
  nsRefPtr<TextRecognition> object = new TextRecognition(win);
  return object.forget();
}

nsISupports*
TextRecognition::GetParentObject() const
{
  return GetOwner();
}

void
TextRecognition::GetLang(nsString& aRetVal) const
{
  aRetVal = mLang;
}

void
TextRecognition::SetLang(const nsAString& aArg)
{
  mLang = aArg;
}

class TextEventRunnable : public nsRunnable
{
public:
  TextEventRunnable(TextRecognition* aText, const nsAString& aRecognitized)
    : mText(aText)
    , mRecognitized(aRecognitized)
  {
  }

  NS_IMETHOD Run() override
  {
    nsresult rv;
    // Create DOM event
    TextRecognitizedEventInit init;
    init.mRecognitizedText = mRecognitized;

    nsRefPtr<TextRecognitizedEvent> event =
        TextRecognitizedEvent::Constructor(mText, NS_LITERAL_STRING("textrecognitized"), init);

    if (NS_WARN_IF(!event)) {
     return NS_ERROR_FAILURE;
    }

    event->SetTrusted(true);
    nsEventStatus dummy = nsEventStatus_eIgnore;
    rv = mText->DispatchDOMEvent(nullptr, event, nullptr, &dummy);

    NS_WARN_IF(NS_FAILED(rv));
    return NS_OK;
  }
  TextRecognition* mText;
  nsString mRecognitized;
};

void TextRecognition::GetRecognitizedResult(const nsAString& aResult)
{
  nsRefPtr<TextEventRunnable> runnable = new TextEventRunnable(this, aResult);
  NS_DispatchToMainThread(runnable);
}

void
TextRecognition::Analysis(ImageBitmap& aImage)
{
  nsresult rv;
  mSourceImage = &aImage;
  // TODO: Should pass media stream as argument in the future.
  rv = mRecognitionService->Analysis(this, &aImage);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return;
  }
}

bool
TextRecognition::SetRecognitionService(ErrorResult& aRv)
{
  mRecognitionService = GetTextRecognitionService();

  if (!mRecognitionService) {
    aRv.Throw(NS_ERROR_DOM_INVALID_STATE_ERR);
    return false;
  }
  return true;
}


} // namespace dom
} // namespace mozilla
