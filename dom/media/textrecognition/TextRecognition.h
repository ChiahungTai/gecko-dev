/*
 * TextRecognition.h
 *
 *  Created on: 2015年8月27日
 *      Author: ctai
 */

#ifndef TREE_DOM_MEDIA_TEXTRECOGNITION_TEXTRECOGNITION_H_
#define TREE_DOM_MEDIA_TEXTRECOGNITION_TEXTRECOGNITION_H_

#include "mozilla/DOMEventTargetHelper.h"
#include "mozilla/WeakPtr.h"
#include "nsITextRecognitionService.h"

namespace mozilla {
namespace dom {

class ImageBitmap;

class TextRecognition final : public DOMEventTargetHelper,
                              public SupportsWeakPtr<TextRecognition>
{
public:
  MOZ_DECLARE_WEAKREFERENCE_TYPENAME(TextRecognition)
  explicit TextRecognition(nsPIDOMWindow* aOwnerWindow);

  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(TextRecognition, DOMEventTargetHelper)

  IMPL_EVENT_HANDLER(textrecognitized)

  nsISupports* GetParentObject() const;

  virtual JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  static already_AddRefed<TextRecognition>
  Constructor(const GlobalObject& aGlobal, ErrorResult& aRv);

  void GetLang(nsString& aRetVal) const;

  void SetLang(const nsAString& aArg);

  void Analysis(ImageBitmap& image);

  void GetRecognitizedResult(const nsAString& aResult);

private:
  virtual ~TextRecognition();
  bool SetRecognitionService(ErrorResult& aRv);

  nsString mLang;
  nsCOMPtr<nsITextRecognitionService> mRecognitionService;
  // Fixme: This is a work around. We should not need it.
  nsRefPtr<ImageBitmap> mSourceImage;
};

} // namespace dom
} // namespace mozilla
#endif /* TREE_DOM_MEDIA_TEXTRECOGNITION_TEXTRECOGNITION_H_ */
