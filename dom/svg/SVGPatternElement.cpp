/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/SVGPatternElement.h"

#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/ArrayUtils.h"
#include "mozilla/dom/SVGLengthBinding.h"
#include "mozilla/dom/SVGPatternElementBinding.h"
#include "mozilla/dom/SVGUnitTypesBinding.h"
#include "DOMSVGAnimatedTransformList.h"
#include "nsGkAtoms.h"

NS_IMPL_NS_NEW_SVG_ELEMENT(Pattern)

namespace mozilla::dom {

using namespace SVGUnitTypes_Binding;

JSObject* SVGPatternElement::WrapNode(JSContext* aCx,
                                      JS::Handle<JSObject*> aGivenProto) {
  return SVGPatternElement_Binding::Wrap(aCx, this, aGivenProto);
}

//--------------------- Patterns ------------------------

SVGElement::LengthInfo SVGPatternElement::sLengthInfo[4] = {
    {nsGkAtoms::x, 0, SVGLength_Binding::SVG_LENGTHTYPE_NUMBER,
     SVGContentUtils::X},
    {nsGkAtoms::y, 0, SVGLength_Binding::SVG_LENGTHTYPE_NUMBER,
     SVGContentUtils::Y},
    {nsGkAtoms::width, 0, SVGLength_Binding::SVG_LENGTHTYPE_NUMBER,
     SVGContentUtils::X},
    {nsGkAtoms::height, 0, SVGLength_Binding::SVG_LENGTHTYPE_NUMBER,
     SVGContentUtils::Y},
};

SVGElement::EnumInfo SVGPatternElement::sEnumInfo[2] = {
    {nsGkAtoms::patternUnits, sSVGUnitTypesMap,
     SVG_UNIT_TYPE_OBJECTBOUNDINGBOX},
    {nsGkAtoms::patternContentUnits, sSVGUnitTypesMap,
     SVG_UNIT_TYPE_USERSPACEONUSE}};

SVGElement::StringInfo SVGPatternElement::sStringInfo[2] = {
    {nsGkAtoms::href, kNameSpaceID_None, true},
    {nsGkAtoms::href, kNameSpaceID_XLink, true}};

//----------------------------------------------------------------------
// Implementation

SVGPatternElement::SVGPatternElement(
    already_AddRefed<mozilla::dom::NodeInfo>&& aNodeInfo)
    : SVGPatternElementBase(std::move(aNodeInfo)) {}

//----------------------------------------------------------------------
// nsINode method

NS_IMPL_ELEMENT_CLONE_WITH_INIT(SVGPatternElement)

//----------------------------------------------------------------------

already_AddRefed<SVGAnimatedRect> SVGPatternElement::ViewBox() {
  return mViewBox.ToSVGAnimatedRect(this);
}

bool SVGPatternElement::IsAttributeMapped(const nsAtom* aAttribute) const {
  return aAttribute == nsGkAtoms::patternTransform ||
         SVGPatternElementBase::IsAttributeMapped(aAttribute);
}

already_AddRefed<DOMSVGAnimatedPreserveAspectRatio>
SVGPatternElement::PreserveAspectRatio() {
  return mPreserveAspectRatio.ToDOMAnimatedPreserveAspectRatio(this);
}

//----------------------------------------------------------------------

already_AddRefed<DOMSVGAnimatedEnumeration> SVGPatternElement::PatternUnits() {
  return mEnumAttributes[PATTERNUNITS].ToDOMAnimatedEnum(this);
}

already_AddRefed<DOMSVGAnimatedEnumeration>
SVGPatternElement::PatternContentUnits() {
  return mEnumAttributes[PATTERNCONTENTUNITS].ToDOMAnimatedEnum(this);
}

already_AddRefed<DOMSVGAnimatedTransformList>
SVGPatternElement::PatternTransform() {
  // We're creating a DOM wrapper, so we must tell GetAnimatedTransformList
  // to allocate the DOMSVGAnimatedTransformList if it hasn't already done so:
  return DOMSVGAnimatedTransformList::GetDOMWrapper(
      GetAnimatedTransformList(DO_ALLOCATE), this);
}

already_AddRefed<DOMSVGAnimatedLength> SVGPatternElement::X() {
  return mLengthAttributes[ATTR_X].ToDOMAnimatedLength(this);
}

already_AddRefed<DOMSVGAnimatedLength> SVGPatternElement::Y() {
  return mLengthAttributes[ATTR_Y].ToDOMAnimatedLength(this);
}

already_AddRefed<DOMSVGAnimatedLength> SVGPatternElement::Width() {
  return mLengthAttributes[ATTR_WIDTH].ToDOMAnimatedLength(this);
}

already_AddRefed<DOMSVGAnimatedLength> SVGPatternElement::Height() {
  return mLengthAttributes[ATTR_HEIGHT].ToDOMAnimatedLength(this);
}

already_AddRefed<DOMSVGAnimatedString> SVGPatternElement::Href() {
  return mStringAttributes[HREF].IsExplicitlySet()
             ? mStringAttributes[HREF].ToDOMAnimatedString(this)
             : mStringAttributes[XLINK_HREF].ToDOMAnimatedString(this);
}

//----------------------------------------------------------------------
// SVGElement methods

SVGAnimatedTransformList* SVGPatternElement::GetAnimatedTransformList(
    uint32_t aFlags) {
  if (!mPatternTransform && (aFlags & DO_ALLOCATE)) {
    mPatternTransform = MakeUnique<SVGAnimatedTransformList>();
  }
  return mPatternTransform.get();
}

/* virtual */
bool SVGPatternElement::HasValidDimensions() const {
  return mLengthAttributes[ATTR_WIDTH].IsExplicitlySet() &&
         mLengthAttributes[ATTR_WIDTH].GetAnimValInSpecifiedUnits() > 0 &&
         mLengthAttributes[ATTR_HEIGHT].IsExplicitlySet() &&
         mLengthAttributes[ATTR_HEIGHT].GetAnimValInSpecifiedUnits() > 0;
}

SVGElement::LengthAttributesInfo SVGPatternElement::GetLengthInfo() {
  return LengthAttributesInfo(mLengthAttributes, sLengthInfo,
                              std::size(sLengthInfo));
}

SVGElement::EnumAttributesInfo SVGPatternElement::GetEnumInfo() {
  return EnumAttributesInfo(mEnumAttributes, sEnumInfo, std::size(sEnumInfo));
}

SVGAnimatedViewBox* SVGPatternElement::GetAnimatedViewBox() {
  return &mViewBox;
}

SVGAnimatedPreserveAspectRatio*
SVGPatternElement::GetAnimatedPreserveAspectRatio() {
  return &mPreserveAspectRatio;
}

SVGElement::StringAttributesInfo SVGPatternElement::GetStringInfo() {
  return StringAttributesInfo(mStringAttributes, sStringInfo,
                              std::size(sStringInfo));
}

}  // namespace mozilla::dom
