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

  nsISupports* GetParentObject() const;

  virtual JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  static already_AddRefed<TextRecognition>
  Constructor(const GlobalObject& aGlobal, ErrorResult& aRv);

  void GetLang(nsString& aRetVal) const;

  void SetLang(const nsAString& aArg);

  void Analysis(ImageBitmap& image);

private:
  virtual ~TextRecognition(){}

  nsString mLang;
};

} // namespace dom
} // namespace mozilla
#endif /* TREE_DOM_MEDIA_TEXTRECOGNITION_TEXTRECOGNITION_H_ */
