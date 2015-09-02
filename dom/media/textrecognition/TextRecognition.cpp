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
GetGestureRecognitionService()
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

void TextRecognition::Analysis(ImageBitmap& aImage)
{

}

} // namespace dom
} // namespace mozilla
