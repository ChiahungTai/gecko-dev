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

#include "opencv2/core/utility.hpp"
#include "opencv2/text.hpp"

#include <string>

using std::string;

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

  //Draw ER's in an image via floodFill
  void er_draw(std::vector<cv::Mat> &channels,
               std::vector<std::vector<cv::text::ERStat> > &regions,
               std::vector<cv::Vec2i> group, cv::Mat& segmentation);
  bool isRepetitive(const string& s);

  nsString mLang;
};

} // namespace dom
} // namespace mozilla
#endif /* TREE_DOM_MEDIA_TEXTRECOGNITION_TEXTRECOGNITION_H_ */
