/*
* $Id: element.cpp,v 1.9 2002-08-28 02:07:40 jlatone Exp $
*
* Maintain the components of each element declaration
*/

/*---------+
| Includes |
*---------*/
#include "../syntax/dlmlist.h"
#include "element.h"

Element const Element::Nil;
ElmContent const ElmContent::Nil;
ElmContent const ElmContent::Undefined(ModelTree(0));
Tag const Tag::Nil;

/*--------------------------------------------------------ElmContent::Rep::Rep-+
| Constructor                                                                  |
+-----------------------------------------------------------------------------*/
ElmContent::Rep::Rep(
   ModelTree const & mdlArg,
   TpIxList const & inclusionsArg,
   TpIxList const & exclusionsArg,
   bool & isOk
) :
   inclusions(inclusionsArg),
   exclusions(exclusionsArg),
   mdl(mdlArg)
{
   iReferenceCount = 0;
   if (
      (inclusions.count() != inclusionsArg.count()) ||
      (exclusions.count() != exclusionsArg.count()) ||
      (mdl.status() == ModelTree::ERROR_NO_MEMORY)
   ) {
      isOk = false;
   }else {
      isOk = true;
   }
}

/*------------------------------------------------------ElmContent::ElmContent-+
| Regular constructors                                                         |
+-----------------------------------------------------------------------------*/
ElmContent::ElmContent(
   ModelTree const & model,
   TpIxList const & inclusions,
   TpIxList const & exclusions
) {
   bool isOk;
   pRep = new Rep(model, inclusions, exclusions, isOk);
   if (!isOk) {
      delete pRep;
      pRep = 0;
   }else {
      ++pRep->iReferenceCount;
   }
}
ElmContent::ElmContent(
   ModelTree const & model
) {
   bool isOk;
   TpIxList exceptionsNil;
   pRep = new Rep(model, exceptionsNil, exceptionsNil, isOk);
   if (!isOk) {
      delete pRep;
      pRep = 0;
   }else {
      ++pRep->iReferenceCount;
   }
}

/*------------------------------------------------------ElmContent::ElmContent-+
| Constructor to reset indices                                                 |
+-----------------------------------------------------------------------------*/
ElmContent::ElmContent(
   ModelTree const & model,           // do have shallow
   TpIxList const & inclusions,       // have no shallow (only deep)
   TpIxList const & exclusions,       // have no shallow (only deep)
   TpNameIxXtable const & ixtbl
) {
   bool isOk;
   pRep = new Rep(model, inclusions, exclusions, isOk);
   if (!isOk) {
      delete pRep;
      pRep = 0;
   }else {
      ++pRep->iReferenceCount;
      pRep->inclusions.resetIndices(ixtbl);
      pRep->exclusions.resetIndices(ixtbl);
   }
}

/*------------------------------------------------------------ElmContent::copy-+
| Deep copy - reset indices                                                    |
+-----------------------------------------------------------------------------*/
ElmContent ElmContent::copy(TpNameIxXtable const & ixtbl) const
{
   return ElmContent(
      pRep->mdl.copy(ixtbl), pRep->inclusions, pRep->exclusions, ixtbl
   );
}

/*------------------------------------------------------ElmContent::operator==-+
| Equality                                                                     |
+-----------------------------------------------------------------------------*/
bool ElmContent::operator==(ElmContent const & source) const {
   if (
      (pRep == source.pRep) || (
         pRep && source.pRep &&
         (pRep->mdl == source.pRep->mdl) &&
         (pRep->inclusions == source.pRep->inclusions) &&
         (pRep->exclusions == source.pRep->exclusions)
      )
   ) {
      return true;
   }else {
      return false;
   }
}

/*------------------------------------------------------------Element::Element-+
| Constructors                                                                 |
+-----------------------------------------------------------------------------*/
Element::Element(
   UnicodeString const & ucsGi,
   int ixElmArg,
   ElmContent const & contentArg,
   bool omitStart,
   bool omitEnd
) :
   RefdKeyRep(ucsGi), ixElm(ixElmArg), content(contentArg)
{
   if (omitStart) b.omitStart = 1; else b.omitStart = 0;
   if (omitEnd) b.omitEnd = 1; else b.omitEnd = 0;
}
Element::Element(
   UnicodeString const & ucsGi,
   int ixElmArg,
   ElmContent const & contentArg,
   bool omitStart,
   bool omitEnd,
   Attlist const & attlstArg
) :
   RefdKeyRep(ucsGi), ixElm(ixElmArg), content(contentArg), attlst(attlstArg)
{
   if (omitStart) b.omitStart = 1; else b.omitStart = 0;
   if (omitEnd) b.omitEnd = 1; else b.omitEnd = 0;
}

/*---------------------------------------------------------------Element::copy-+
| Deep copy                                                                    |
+-----------------------------------------------------------------------------*/
Element Element::copy(TpNameIxXtable const & ixtbl) const
{
   int ix = ixtbl.inqIx(key());
   return Element(
      ixtbl.inqName(ix),
      ix,
      content.copy(ixtbl),
      b.omitStart,
      b.omitEnd,
      attlst.copy()
   );
}

/*---------------------------------------------------------Element::operator==-+
| Equality                                                                     |
+-----------------------------------------------------------------------------*/
bool Element::operator==(Element const & source) const
{
   if (
      (content == source.content) &&
      (attlst == source.attlst) &&
      (b.omitStart == source.b.omitStart) &&
      (b.omitEnd == source.b.omitEnd)
   ) {
      return true;
   }else {
      return false;
   }
}

/*------------------------------------------------------Element::attachContent-+
|                                                                              |
+-----------------------------------------------------------------------------*/
void Element::attachContent(
   ElmContent const & contentArg,
   bool omitStart,
   bool omitEnd
) {
   content = contentArg;
   if (omitStart) b.omitStart = 1; else b.omitStart = 0;
   if (omitEnd) b.omitEnd = 1; else b.omitEnd = 0;
}

/*------------------------------------------------------Element::attachAttlist-+
| Attach an attlst to a given element                                          |
| Return false if a non-empty attlst was already attached                      |
+-----------------------------------------------------------------------------*/
bool Element::attachAttlist(Attlist const & attlstArg) {
   if (!attlst.count()) {
      attlst = attlstArg;
      return true;
   }else {
      return false;
   }
}

/*------------------------------------------------ostream& operator<<(Element)-+
| Display the contents of an Element                                           |
+-----------------------------------------------------------------------------*/
ostream & operator<<(ostream& out, Element const & elm)
{
#if !defined MWERKS_NEEDS_Q_FIX  // formatted io
   out << '\"' << elm.inqName() << "\" #" << elm.inqIx() << endl;
   out << "   Model:" << endl;
   out.width(6);
   out << elm.inqModel() << endl;
   // this is not completely finished...
#endif
   return out;
}

/*--------------------------------------------------------Tag::checkEndElement-+
| Effects:                                                                     |
|   See if an end tag is valid by checking if it matches any open element      |
|   in its ancestry.                                                           |
| Returns:                                                                     |
|    >0 depth of the first open element that matches                           |
|     0 no element matches                                                     |
+-----------------------------------------------------------------------------*/
int Tag::checkEndElement(Element const * pElm) const
{
   int iToEnd = 1;
   Tag const * pTag = this;
   while ((Element const *)pTag->elmptr.inqData() != pElm) {
      if (pTag = (Tag const *)pTag->parent.inqData(), !pTag) return 0;
      ++iToEnd;
   }
   return iToEnd;
}

/*------------------------------------------------------------Tag::checkEndNet-+
| Effects:                                                                     |
|   See if a NULL end-tag has a matching null enabling start in the ancestry   | |
| Returns:                                                                     |
|    >0 depth of the first open element that matches                           |
|     0 no element matches                                                     |
+-----------------------------------------------------------------------------*/
int Tag::checkEndNet() const
{
   int iToEnd = 1;
   Tag const * pTag = this;
   while (pTag->flag.miniStart != Net) {
      if (pTag = (Tag const *)pTag->parent.inqData(), !pTag) return 0;
      ++iToEnd;
   }
   return iToEnd;
}

/*--------------------------------------------------------Tag::checkExceptions-+
| Effects:                                                                     |
|   See if the element is excluded / included by checking if it belongs        |
|   to a plus or minus exception group of any open element in the ancestry.    |
|                                                                              |
|   According to ISO 8879, it first checks in the minus groups (exclusions),   |
|   then, if not found, check in the plus groups of all open elements.         |
|                                                                              |
| Returns:                                                                     |
|    >0 excluded   -- this is also the depth of the first open                 |
|       element that raised the exclusion.  This value can be used to perform  |
|       recovery by closing all elements until this depth.                     |
|    <0 included; otherwise                                                    |
|     0 neither an exclusion. nor an inclusion                                 |
+-----------------------------------------------------------------------------*/
int Tag::checkExceptions(Element const * pElm) const
{
   int i;
   Tag const * pTag;
   for (pTag=this, i=1; pTag; pTag=(Tag const *)pTag->parent.inqData(), ++i) {
      if (((Element const *)pTag->elmptr.inqData())->isExcluded(pElm)) {
         return i;
      }
   }
   for (pTag=this, i=1; pTag; pTag=(Tag const *)pTag->parent.inqData(), ++i) {
      if (((Element const *)pTag->elmptr.inqData())->isIncluded(pElm)) {
         return -i;
      }
   }
   return 0;
}

/*------------------------------------------------------------Tag::inqLongName-+
| Recursive to get the tag names in the right order                            |
+-----------------------------------------------------------------------------*/
bool Tag::inqLongName(Writer & umost) const
{
   if ((!elmptr.isPresent()) || (!parent.isPresent())) {
      // 2nd condition is to avoid printing #ROOT
      return false;
   }else {
      if (inqParent().inqLongName(umost)) umost << ',';
      umost << elmptr.key();
      return true;
   }
}
UnicodeString Tag::inqLongName() const
{
   UnicodeMemWriter umost;
   if (!inqLongName(umost)) {
      return UnicodeString::Nil;
   }else {
      return umost;
   }
}

/*-------------------------------------------------------------Tag::stringize-+
| Write the tag in a normalized form, using the ref concrete syntax.          |
+----------------------------------------------------------------------------*/
void Tag::stringize(
   Writer & uost,
   bool isEndTag,
   bool isFullAttlist
) const {
   if (isEndTag) {
      uost << DelimList::rcsValue(Delimiter::IX_ETAGO) << inqElement().key();
   }else {
      uost << DelimList::rcsValue(Delimiter::IX_STAGO) << inqElement().key();
      attlst.stringize(uost, isFullAttlist);
   }
   uost << DelimList::rcsValue(Delimiter::IX_TAGC);
}

/*============================================================================*/
