/*
* $Id: yspevent.cpp,v 1.12 2002-04-01 10:21:23 pgr Exp $
*
* Events and stack of events to later report to the Application
*/

/*---------+
| Includes |
+---------*/
#include "yspenum.h"
#include "yspevent.h"
#include "../../reslib/resources.h"
#include "../../toolslib/uccompos.h"
#include "../syntax/dlmlist.h"
#include "../element/element.h"
#include "../entity/entity.h"

YaspEvent const YaspEvent::Nil;

/*----------------------------------------------------------YaspEvent::accept-+
| Implementations Notes (PGR, 07/02/96 updated 09/26/97)                      |
|   "accept" should be a virtual.  However:                                   |
|   - copying events is a fundamental feature of Yasp.                        |
|     C++ has no virtual constructors, so to copy events requires tricks:     |
|     assume all events have the same length, copy virtual fct table, etc...  |
|     Also most compilers base their optimization on the                      |
|     "no virtual constructor" assumption.                                    |
|     i.e.  Animal animal(dog) does not make the dog barks.                   |
|   - it resulted in a x-uplication of YaspEventXxx objects, tedious and      |
|     harder to maintain                                                      |
|   Implementing "accept" as a virtual has been abandonned.                   |
+----------------------------------------------------------------------------*/
void YaspEvent::accept(YaspEventInspector & inspector) const
{
   switch (tType) {
   case YSP__DCL_START:
      inspector.inspectDtdStart(*(Entity const *)inqData());
      break;
   case YSP__DCL_ELEMENT:
      inspector.inspectDclElement(*(Element const *)inqData());
      break;
   case YSP__DCL_ATTRIBUTE:
      inspector.inspectDclElementAttlist(*(Element const *)inqData());
      break;
   case YSP__DCL_PENTITY:
   case YSP__DCL_ENTITY:
      inspector.inspectDclEntity(*(Entity const *)inqData());
      break;
   case YSP__DCL_NOTATION:
      inspector.inspectDclNotation(*(Notation const *)inqData());
      break;
   case YSP__DCL_NATTRIBUTE:
      inspector.inspectDclNotationAttlist(*(Notation const *)inqData());
      break;
   case YSP__TAG_START:
      inspector.inspectTagStart(*(Tag const *)inqData());
      break;
   case YSP__TAG_END:
      inspector.inspectTagEnd(*(Tag const *)inqData());
      break;
   case YSP__TAG_EMPTY:
      inspector.inspectTagEmpty(*(Tag const *)inqData());
      break;
   case YSP__TEXT:
      inspector.inspectText(*(TextData const *)inqData());
      break;
   case YSP__SDATA_TEXT:
      inspector.inspectSdataText(*(TextData const *)inqData());
      break;
   case YSP__CDATA_TEXT:
      inspector.inspectCdataText(*(TextData const *)inqData());
      break;
   case YSP__PI_TEXT:
      inspector.inspectPiText(*(TextData const *)inqData());
      break;
   case YSP__COMMENT_TEXT:
      inspector.inspectCommentText(*(TextData const *)inqData());
      break;
   case YSP__ENTITY_REF:
      inspector.inspectEntityRef(*(Entity const *)inqData());
      break;
   case YSP__ENTITY_START:
   case YSP__PENTITY_START:
   case YSP__SDATA_START:
   case YSP__CDATA_START:
   case YSP__NDATA_START:
   case YSP__ENT_PI_START:
   case YSP__SUBDOC_START:
      inspector.inspectEntityStart(*(Entity const *)inqData());
      break;
   case YSP__ENTITY_END:
   case YSP__SDATA_END:
   case YSP__CDATA_END:
   case YSP__NDATA_END:
   case YSP__ENT_PI_END:
   case YSP__SUBDOC_END:
      inspector.inspectEntityEnd(*(Entity const *)inqData());
      break;
   case YSP__ENTITY_CANT:
      {
         Entity const * pEnt = (Entity const *)inqData();
         if (!pEnt) pEnt = &Entity::Nil;
         inspector.inspectEntityCantOpen(*pEnt);
      }
      break;
   case YSP__MARKED_START:
      inspector.inspectMarkedStart(*(MarkedSectionData const *)inqData());
      break;
   case YSP__XML_PI:
      inspector.inspectXmlPi(*(XmlPiData const *)inqData());
      break;
   case YSP__PI_START:
      inspector.inspectPiStart();
      break;
   case YSP__PI_END:
      inspector.inspectPiEnd();
      break;
   case YSP__COMMENT_START:
      inspector.inspectCommentStart();
      break;
   case YSP__COMMENT_END:
      inspector.inspectCommentEnd();
      break;
   case YSP__DCL_END:
      inspector.inspectDtdEnd();
      break;
   case YSP__DCL_NULL:
      inspector.inspectDclNull();
      break;
   case YSP__MARKED_END:
      inspector.inspectMarkedEnd();
      break;
   case YSP__PARSE_START:
      inspector.inspectParseStart();
      break;
   case YSP__PARSE_END:
      inspector.inspectParseEnd();
      break;
   case YSP__ERROR:
      inspector.inspectError();
      break;
   default: // case YSP__ERROR_FATAL:
      inspector.inspectErrorFatal();
      break;
   }
}

/*-------------------------------------------------------YaspEventList::reset-+
|                                                                             |
+----------------------------------------------------------------------------*/
void YaspEventList::reset() {
   while (tlst.count()) {
      rlst.removeLast();
      tlst.removeLast();
   }
}

/*--------------------------------------------------------YaspEvent::makeCopy-+
| TextData events are copied in a specific way.  For performance reasons,     |
| there is only one buffer that contains the textual data for the whole       |
| parsing of the document.  This buffer belongs to a UnicodeStringFlash,      |
| which is a member of the TextData event.                                    |
| If TextData events were simply copied by reference counting, then their     |
| data would change as the parse goes along.  The solution was to copy        |
| the UnicodeString rather than to copy the event.  The mechanism for         |
| copying UnicodeStringFlash is safe -- despite what the comment says         |
| in ucstring.cpp.  At the first time, the copy will create a                 |
| not-so-flashy UnicodeStringFlash. (See the shallow copy of UnicodeString.)  |
| "not-so-flashy" means:                                                      |
| -  the UnicodeStringFlash will now behave like a true UnicodeString         |
|   (refCounted and so on...)                                                 |
| - its buffer is separate from the original TextData buffer (deep            |
|   copy)                                                                     |
|                                                                             |
| This is what it would lokks like if a given TextEvent data was copied       |
| twice:                                                                      |
|                                                                             |
|    Yasp-owned TextData Event:      ref=1,                                   |
|                                    ucstring_buf=UniqueBuf,                  |
|                                    ucstring_ref=1                           |
|                                                                             |
|    Copy #1 of the TextData Event:  ref=1,                                   |
|                                    ucstring_buf=AnotherBuf,                 |
|                                    ucstring_ref=2                           |
|                                                                             |
|    Copy #2 of the TextData Event:  ref=1,                                   |
|                                    ucstring_buf=AnotherBuf,                 |
|                                    ucstring_ref=2                           |
|                                                                             |
|                                                                             |
| Note that the method Yasp3::setTextData() permits to control the buffer     |
| of the yasp-owned TextData event.  It implicitely means that the            |
| programmer will not issue any event copies, but just gets its own copies    |
| of the relevant data at the time the TextData event is reported.            |
| Yes: this is dangerous, but required when performance matters.              |
+----------------------------------------------------------------------------*/
RefdItemRep * YaspEvent::makeCopy() const
{
   switch (tType) {
   case YSP__TEXT:
   case YSP__COMMENT_TEXT:
   case YSP__CDATA_TEXT:
   case YSP__SDATA_TEXT:
   case YSP__PI_TEXT:
      return new TextData(((TextData const *)this->inqData())->inqText());
   default:
      return (RefdItemRep *)this->inqData();
   }
}

/*---------------------------------------------------- Event Content Inquire -+
| Try to use these functions sparingly                                        |
| they are more C'ish than C++, and then error-prone,                         |
+----------------------------------------------------------------------------*/
Tag const * YaspEvent::inqTag() const
{
   switch (tType) {
   case YSP__TAG_START:
   case YSP__TAG_END:
   case YSP__TAG_EMPTY:
      return (Tag const *)inqData();
   default:
      return 0;
   }
}
TextData const * YaspEvent::inqTextData() const
{
   switch (tType) {
   case YSP__TEXT:
   case YSP__COMMENT_TEXT:
   case YSP__SDATA_TEXT:
   case YSP__CDATA_TEXT:
   case YSP__PI_TEXT:
      return (TextData const *)inqData();
   default:
      return 0;
   }
}
Element const * YaspEvent::inqElement() const
{
   switch (tType) {
   case YSP__DCL_ELEMENT:
   case YSP__DCL_ATTRIBUTE:
      return (Element const *)inqData();
   default:
      return 0;
   }
}
Entity const * YaspEvent::inqEntity() const
{
   switch (tType) {
   case YSP__DCL_START:
   case YSP__DCL_PENTITY:
   case YSP__DCL_ENTITY:
   case YSP__ENTITY_REF:
   case YSP__ENTITY_START:
   case YSP__PENTITY_START:
   case YSP__CDATA_START:
   case YSP__SDATA_START:
   case YSP__NDATA_START:
   case YSP__ENT_PI_START:
   case YSP__SUBDOC_START:
   case YSP__ENTITY_END:
   case YSP__CDATA_END:
   case YSP__SDATA_END:
   case YSP__NDATA_END:
   case YSP__ENT_PI_END:
   case YSP__SUBDOC_END:
   case YSP__ENTITY_CANT:
      return (Entity const *)inqData();
   default:
      return 0;
   }
}
Notation const * YaspEvent::inqNotation() const
{
   switch (tType) {
   case YSP__DCL_NOTATION:
   case YSP__DCL_NATTRIBUTE:
      return (Notation const *)inqData();
   default:
      return 0;
   }
}
MarkedSectionData const * YaspEvent::inqMarkedSectionData() const {
   if (tType == YSP__MARKED_START) {
      return (MarkedSectionData const *)inqData();
   }else {
      return 0;
   }
}
XmlPiData const * YaspEvent::inqXmlPiData() const {
   if (tType == YSP__XML_PI) {
      return (XmlPiData const *)inqData();
   }else {
      return 0;
   }
}

/*-----------------------------------------------------YaspEvent::inqTypeName-+
| Effects:                                                                    |
|   Convert a numeric YaspType to the equivalent name or description.         |
|   Mainly for print info in test drivers, messages, etc.                     |
+----------------------------------------------------------------------------*/
char const * YaspEvent::inqTypeName(YaspType type)
{
   switch (type) {
   case YSP__TAG_START:       return "Element start";
   case YSP__TAG_END:         return "Element end";
   case YSP__TAG_EMPTY:       return "Empty element";
   case YSP__TEXT:            return "Text";
   case YSP__ENTITY_REF:      return "Unresolved entity";
   case YSP__ENTITY_START:    return "Entity start";
   case YSP__ENTITY_END:      return "Entity end";
   case YSP__ENTITY_CANT:     return "Can't open entity";
   case YSP__ERROR:           return "Parsing error";
   case YSP__PARSE_START:     return "Start of document";
   case YSP__PARSE_END:       return "End of document";
   case YSP__DCL_PENTITY:     return "Parameter entity decl.";
   case YSP__DCL_ENTITY:      return "General entity decl.";
   case YSP__DCL_ELEMENT:     return "Element decl.";
   case YSP__DCL_ATTRIBUTE:   return "Attribute decl.";
   case YSP__DCL_NOTATION:    return "Notation decl.";
   case YSP__DCL_NULL:        return "Null markup decl.";
   case YSP__COMMENT_START:   return "Comment start";
   case YSP__COMMENT_TEXT:    return "Comment text";
   case YSP__COMMENT_END:     return "Comment end";
   case YSP__MARKED_START:    return "Marked section start";
   case YSP__MARKED_END:      return "Marked section end";
   case YSP__DCL_START:       return "DOCTYPE decl. start";
   case YSP__DCL_END:         return "DOCTYPE decl. end";
   case YSP__PI_START:        return "PI start";
   case YSP__PI_END:          return "PI end";
   case YSP__NDATA_START:     return "NDATA start";
   case YSP__NDATA_END:       return "NDATA end";
   case YSP__SDATA_START:     return "SDATA start";
   case YSP__SDATA_END:       return "SDATA end";
   case YSP__XML_PI:          return "XML PI";
   // case YSP__SUBDOC_START:
   // case YSP__SUBDOC_END:
   // case YSP__DCL_ELEMENT_IMPLIED:
   default:
      break;
   }
   return "Unknown Token Type";
}

/*----------------------------------------YaspEventList__Validator inspectors-+
|                                                                             |
+----------------------------------------------------------------------------*/
void YaspEventList__Validator::inspectTagStart(Tag const & tag)
{
   if (tag.isOmittedStart() && tag.inqElement().isStartRequired()) {
      erh << ECE__ERROR << _YSP__INVSTGMINI << tag.inqElement().key() << endm;
   }
}
void YaspEventList__Validator::inspectTagEnd(Tag const & tag)
{
   if (tag.isInvalidEnd()) {
      erh << _YSP__INVETGPREMA;
   }else if (tag.isOmittedEnd() && tag.inqElement().isEndRequired()) {
      erh << _YSP__INVETGMINI;
   }else {
      return;
   }
   erh << ECE__ERROR << tag.inqElement().key() << endm;
}

/*---------------------------------------------------YaspEventText::setBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
void YaspEventText::setBuffer(UCS_2 * pUcText)
{
   ((TextData *)inqData())->setBuffer(pUcText);
}

/*-------------------------------------------------------XmlPiData::stringize-+
| Stringize a YSP_XML_PI event.                                               |
+----------------------------------------------------------------------------*/
void XmlPiData::stringize(Writer & uost) const
{
   uost
      << DelimList::rcsValue(Delimiter::IX_PIO)
      << "XML "
      << "V=" << ucsVersion
      << ", E=" << ucsEncoding
      << ", R=" << ucsRmd;
}

/*-----------------------------------------------------MarkedSectionData::add-+
|                                                                             |
+----------------------------------------------------------------------------*/
void MarkedSectionData::add(Entity const * pEnt, e_MsKey key)
{
   if (pEnt) {                // if it comes from a parm ent
      if (iEntCount < (sizeof aEnt / sizeof aEnt[0])) {
         aEnt[iEntCount++] = pEnt;
      }
   }else {
      if (iKeyCount < (sizeof aKey / sizeof aKey[0])) {
         aKey[iKeyCount++] = key;
      }
   }
}

/*-----------------------------------------------MarkedSectionData::stringize-+
| Stringize a YSP__MARKED_START event.                                        |
+----------------------------------------------------------------------------*/
void MarkedSectionData::stringize(
   Writer & uost
) const {
   /*
   | First, output the resulting key
   */
   uost << ReservedName::rcsName(resultingKey);
   uost << (UCS_2)0;
   int i;

   /*
   | for all keys in aKey[], output a list of values
   | separated by a space
   */
   for (i=0; i < iKeyCount; ++i) {
       if (i) uost << (UCS_2)' ';           // cont'd
       uost << ReservedName::rcsName(aKey[i]);
   }
   uost << (UCS_2)0;
   /*
   | for all entities in aEnt[], output a list of entname's
   | separated by a space
   */
   for (i=0; i < iEntCount; ++i) {
       if (i) uost << (UCS_2)' ';           // cont'd
       uost << (aEnt[i])->inqName();
   }
   uost << (UCS_2)0;
}


/*--------------------------------------------------YaspStringizer inspectors-+
|                                                                             |
+----------------------------------------------------------------------------*/
void YaspStringizer::inspectTagStart(Tag const & tag) {
   tag.stringize(
      m_uost,
      false,
      m_isFullAttlist
   );
}
void YaspStringizer::inspectTagEnd(Tag const & tag) {
   tag.stringize(
      m_uost,
      true,
      m_isFullAttlist
   );
}

void YaspStringizer::inspectTagEmpty(Tag const & tag) {
   tag.stringize(
      m_uost,
      false,
      m_isFullAttlist
   );
}

void YaspStringizer::inspectMarkedStart(MarkedSectionData const & msd) {
   msd.stringize(m_uost);
}

void YaspStringizer::inspectText(TextData const & txt) {
   m_uost << txt.inqText();
}

void YaspStringizer::inspectPiText(TextData const & txt) {
   m_uost << txt.inqText();
}

void YaspStringizer::inspectCdataText(TextData const & txt) {
   m_uost << txt.inqText();
}

void YaspStringizer::inspectSdataText(TextData const & txt) {
   m_uost << txt.inqText();
}

void YaspStringizer::inspectCommentText(TextData const & txt) {
   m_uost << txt.inqText();
}

void YaspStringizer::inspectEntityStart(Entity const & ent) {
   ent.stringize(YSP__noType, m_uost);
}

void YaspStringizer::inspectEntityRef(Entity const & ent) {
   ent.stringize(YSP__ENTITY_REF, m_uost);
}

void YaspStringizer::inspectEntityEnd(Entity const & ent) {
   m_uost << ent.inqName();
}

void YaspStringizer::inspectDtdStart(Entity const & entDoctype)
{
   ExternId exid = entDoctype.inqExternId();
   m_uost
      << DelimList::rcsValue(Delimiter::IX_MDO)
      << ReservedName::rcsName(ReservedName::IX_DOCTYPE)
      << (UCS_2)' ' << entDoctype.inqName()
      << "' PUBID=" << exid.inqDelimPublicId() << exid.inqPublicId()
      << exid.inqDelimPublicId()
      << " SYSID=" << exid.inqDelimSystemId() << exid.inqSystemId()
      << exid.inqDelimSystemId()
      << DelimList::rcsValue(Delimiter::IX_MDC);
}

void YaspStringizer::inspectDclElement(Element const & elm) {
   m_uost
      << DelimList::rcsValue(Delimiter::IX_MDO)
      << ReservedName::rcsName(ReservedName::IX_ELEMENT)
      << (UCS_2)' ' << elm.inqGi()
      << DelimList::rcsValue(Delimiter::IX_MDC);
}

void YaspStringizer::inspectDclElementAttlist(Element const & elm) {
   m_uost
      << DelimList::rcsValue(Delimiter::IX_MDO)
      << ReservedName::rcsName(ReservedName::IX_ATTLIST)
      << (UCS_2)' ' << elm.inqGi()
      << '[' << elm.inqAttlist().count() << " attributes]";
}

void YaspStringizer::inspectDclEntity(Entity const & ent) {
   if (ent.isDefault()) {
      UnicodeString ucsDefault(
         UnicodeString('#') +
         ReservedName::rcsName(ReservedName::IX_DEFAULT)
      );
      m_uost << ucsDefault;
   }else {
      m_uost << ent.inqName();
   }
   if (ent.isExternal()) {
      ExternId const & exid = ent.inqExternId();
      if (exid.isPublic()) {
         m_uost << ReservedName::rcsName(ReservedName::IX_PUBLIC);
      }else if (exid.isSystem()) {
         m_uost << ReservedName::rcsName(ReservedName::IX_SYSTEM);
      }
      m_uost << exid.inqSystemId();
   }else {
      m_uost << ent.inqLiteral();
   }
}

void YaspStringizer::inspectDclNotation(Notation const & ncb) {
   m_uost
      << ReservedName::rcsName(ReservedName::IX_NOTATION)
      << ncb.inqName()
      << ncb.inqSystemId();
}

void YaspStringizer::inspectDclNotationAttlist(Notation const & ncb) {
   m_uost
      << ReservedName::rcsName(ReservedName::IX_ATTLIST)
      << ncb.inqName()
      << '[' << ncb.inqAttlist().count() << " attributes]";
}

void YaspStringizer::inspectXmlPi(XmlPiData const & xdata) {
   xdata.stringize(m_uost);
}

/*===========================================================================*/
