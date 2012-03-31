/*
* $Id: yspevent.h,v 1.14 2011-07-29 10:26:40 pgr Exp $
*
* This is the letter class for YaspEvent's.
*/

#if !defined YSPEVENT_HEADER && defined __cplusplus
#define YSPEVENT_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include <new>
#include "../../toolslib/tplist.h"
#include "../../toolslib/tplistix.h"
#include "yspenum.h"

class Tag;
class TextData;
class Element;
class Notation;
class Entity;
class UnicodeComposer;
class Writer;
class MarkedSectionData;
class XmlPiData;

/*-----------------------------------YaspeEventInspector::YaspeEventInspector-+
| Inspector to process any reported event                                     |
+----------------------------------------------------------------------------*/
class YASP3_API YaspEventInspector {
public:
   virtual ~YaspEventInspector() {}
   virtual void inspectDtdStart(           Entity const &            ) {}
   virtual void inspectDclElement(         Element const &           ) {}
   virtual void inspectDclElementAttlist(  Element const &           ) {}
   virtual void inspectDclEntity(          Entity const &            ) {}
   virtual void inspectDclNotation(        Notation const &          ) {}
   virtual void inspectDclNotationAttlist( Notation const &          ) {}
   virtual void inspectTagStart(           Tag const &               ) {}
   virtual void inspectTagEnd(             Tag const &               ) {}
   virtual void inspectTagEmpty(           Tag const &               ) {}
   virtual void inspectText(               TextData const &          ) {}
   virtual void inspectSdataText(          TextData const &          ) {}
   virtual void inspectCdataText(          TextData const &          ) {}
   virtual void inspectPiText(             TextData const &          ) {}
   virtual void inspectCommentText(        TextData const &          ) {}
   virtual void inspectEntityRef(          Entity const &            ) {}
   virtual void inspectEntityStart(        Entity const &            ) {}
   virtual void inspectEntityEnd(          Entity const &            ) {}
   virtual void inspectEntityCantOpen(     Entity const &            ) {}
   virtual void inspectMarkedStart(        MarkedSectionData const & ) {}
   virtual void inspectXmlPi(              XmlPiData const &         ) {}
   virtual void inspectPiStart(            /* void */                ) {}
   virtual void inspectPiEnd(              /* void */                ) {}
   virtual void inspectCommentStart(       /* void */                ) {}
   virtual void inspectCommentEnd(         /* void */                ) {}
   virtual void inspectDtdEnd(             /* void */                ) {}
   virtual void inspectDclNull(            /* void */                ) {}
   virtual void inspectMarkedEnd(          /* void */                ) {}
   virtual void inspectParseStart(         /* void */                ) {}
   virtual void inspectParseEnd(           /* void */                ) {}
   virtual void inspectError(              /* void */                ) {}
   virtual void inspectErrorFatal(         /* void */                ) {}
protected:
   YaspEventInspector() {}
};

/*------------------------------------------------------------------YaspEvent-+
|                                                                             |
+----------------------------------------------------------------------------*/
class YASP3_API YaspEvent : public RefdItem {
   friend class YaspEventText;          // internal
public:
   YaspEvent(YaspType tTypeArg = YSP__noType);
   YaspEvent(RefdItemRep const * p, YaspType tTypeArg);
   YaspEvent(YaspEvent const & source);
   YaspEvent & operator=(YaspEvent const & source);

   YaspType inqType() const;
   static char const * inqTypeName(YaspType type);
   void accept(YaspEventInspector & p) const;

   // it's better to use inspectors, but these inquiries are also required
   // for simple cases.
   Tag const * inqTag() const;
   TextData const * inqTextData() const;
   Element const * inqElement() const;
   Entity const * inqEntity() const;
   Notation const * inqNotation() const;
   MarkedSectionData const * inqMarkedSectionData() const;
   XmlPiData const * inqXmlPiData() const;

private:
   RefdItemRep * makeCopy() const;
   YaspType tType;
public:
   static const YaspEvent Nil;
};

/* -- INLINES -- */
inline YaspEvent::YaspEvent(YaspType tTypeArg) : tType(tTypeArg) {
}
inline YaspEvent::YaspEvent(
   RefdItemRep const * p, YaspType tTypeArg
) :
   RefdItem(p), tType(tTypeArg)
{
}
inline YaspEvent::YaspEvent(
   YaspEvent const & source
) :
   RefdItem(source.makeCopy()),
   tType(source.tType)
{
}
inline YaspType YaspEvent::inqType() const {
   return tType;
}
inline YaspEvent & YaspEvent::operator=(YaspEvent const & source) {
   if (inqData() != source.inqData()) {
      this->~YaspEvent();
      new(this) YaspEvent(source);
   }else {
      tType = source.tType;
   }
   return *this;
}

/*-------------------------------------------------------------------TextData-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TextData : public RefdItemRep {
public:
   TextData();
   TextData(UnicodeString const & ucs);
   TextData(UCS_2 const * pUc);

   void setLength(int iLen);
   void setBuffer(UCS_2 * pUc);

   // const methods
   UnicodeString const & inqText() const;
   int inqLength() const;      // for conveniency: inqText().length() is OK
private:
   UcStringFlash ucs;
};

/* -- INLINES -- */
inline TextData::TextData() : ucs(0) {
}
inline TextData::TextData(UnicodeString const & ucsArg) : ucs(ucsArg)
{}
inline TextData::TextData(UCS_2 const * pUc) : ucs(pUc, 0)
{}
inline void TextData::setLength(int iLen) {
   ucs.resetLength(iLen);
}
inline void TextData::setBuffer(UCS_2 * pUc) {
   ucs.resetBuffer(pUc);
}
inline UnicodeString const & TextData::inqText() const {
   return ucs;
}
inline int TextData::inqLength() const {
   return ucs.length();
}

/*--------------------------------------------------------------YaspEventText-+
| Internal class                                                              |
| The purpose of this is to optimize buffer management by reusing the same    |
| buffer and avoiding unnecessary copies (UnicodeString.).                    |
|                                                                             |
| The same buffer is used to output any kind of text (PCDATA, comment, PI's). |
| This buffer can either be internal, or belongs to the Application           |
| (Yasp3::setTextBuffer).                                                     |
+----------------------------------------------------------------------------*/
class YASP3_API YaspEventText : public YaspEvent {
public:
   YaspEventText();
   void setTypeAndLength(YaspType tTypeArg, int iLengthArg);
   void setBuffer(UCS_2 * pUcText);

private:
   YaspEventText& operator=(YaspEventText const& source); // no!
   YaspEventText(YaspEventText const& source);            // no!
};

/* -- INLINES -- */
inline YaspEventText::YaspEventText(
) :
   YaspEvent(new TextData, YSP__TEXT)
{}
inline void YaspEventText::setTypeAndLength(YaspType tTypeArg, int iLen) {
   tType = tTypeArg;
   ((TextData *)inqData())->setLength(iLen);
}

/*----------------------------------------------------------MarkedSectionData-+
| This structure is used associated to inspectMarkedStart()                   |
| It stores the miscellaneous components that made the status of the MS.      |
| (not a very sophisticated class, nor solid, indeed)                         |
+----------------------------------------------------------------------------*/
class MarkedSectionData : public RefdItemRep {
public:
   MarkedSectionData();

   e_MsKey inqResult() const;
   int inqEntityCount() const;
   Entity const * inqEntity(int i) const;
   int inqKeyCount() const;
   e_MsKey inqKey(int i) const;
   void stringize(Writer & uost) const;

   // non-const
   void setResult(e_MsKey key);
   void add(Entity const * pEnt, e_MsKey key);

private:
   MarkedSectionData& operator=(MarkedSectionData const& source); // no!
   MarkedSectionData(MarkedSectionData const& source);            // no!

   int iKeyCount;
   int iEntCount;
   e_MsKey resultingKey;
   e_MsKey aKey[32];
   Entity const * aEnt[32];
};

/* -- INLINES -- */
inline MarkedSectionData::MarkedSectionData(
) :
   iKeyCount(0), iEntCount(0) {
}
inline e_MsKey MarkedSectionData::inqResult() const {
   return resultingKey;
}
inline int MarkedSectionData::inqEntityCount() const {
   return iEntCount;
}
inline Entity const * MarkedSectionData::inqEntity(int i) const {
   if (i < iEntCount) return aEnt[i]; else return 0;
}
inline int MarkedSectionData::inqKeyCount() const {
   return iKeyCount;
}
inline e_MsKey MarkedSectionData::inqKey(int i) const {
   if (i < iKeyCount) return aKey[i]; else return MSKEY_INVALID;
}
inline void MarkedSectionData::setResult(e_MsKey key) {
   resultingKey = key;
}

/*------------------------------------------------------------------XmlPiData-+
| This structure is used associated to inspectXmlPi()                         |
| It stores the miscellaneous components that made the PI.                    |
+----------------------------------------------------------------------------*/
class XmlPiData : public RefdItemRep {
public:
   XmlPiData(
      UnicodeString const & ucsVersionArg,
      UnicodeString const & ucsEncodingArg,
      UnicodeString const & ucsRmdArg,
      UCS_2 ucVersionDlmArg,
      UCS_2 ucEncodingDlmArg,
      UCS_2 ucRmdDlmArg
   );
   UnicodeString const & inqVersion() const;
   UnicodeString const & inqEncoding() const;
   UnicodeString const & inqRequiredMarkup() const;
   UCS_2 inqVersionDelim() const;
   UCS_2 inqEncodingDelim() const;
   UCS_2 inqRmdDelim() const;

   void stringize(Writer & uost) const;
private:
   XmlPiData& operator=(XmlPiData const& source); // no!
   XmlPiData(XmlPiData const& source);            // no!

   UnicodeString ucsVersion;
   UnicodeString ucsEncoding;
   UnicodeString ucsRmd;
   UCS_2 ucVersionDlm;
   UCS_2 ucEncodingDlm;
   UCS_2 ucRmdDlm;
};

/* -- INLINES -- */
inline XmlPiData::XmlPiData(
   UnicodeString const & ucsVersionArg,
   UnicodeString const & ucsEncodingArg,
   UnicodeString const & ucsRmdArg,
   UCS_2 ucVersionDlmArg,
   UCS_2 ucEncodingDlmArg,
   UCS_2 ucRmdDlmArg
) :
   ucsVersion(ucsVersionArg),
   ucsEncoding(ucsEncodingArg),
   ucsRmd(ucsRmdArg),
   ucVersionDlm(ucVersionDlmArg),
   ucEncodingDlm(ucEncodingDlmArg),
   ucRmdDlm(ucRmdDlmArg)
{}
inline UnicodeString const & XmlPiData::inqVersion() const {
   return ucsVersion;
}
inline UnicodeString const & XmlPiData::inqEncoding() const {
   return ucsEncoding;
}
inline UnicodeString const & XmlPiData::inqRequiredMarkup() const {
   return ucsRmd;
}
inline UCS_2 XmlPiData::inqVersionDelim() const {
   return ucVersionDlm;
}
inline UCS_2 XmlPiData::inqEncodingDelim() const {
   return ucEncodingDlm;
}
inline UCS_2 XmlPiData::inqRmdDelim() const {
   return ucRmdDlm;
}

/*--------------------------------------------------------------YaspEventList-+
| FIFO for pending events                                                     |
+----------------------------------------------------------------------------*/
// Validator is not nestable for SGI5
class YASP3_API YaspEventList__Validator : public YaspEventInspector {
public:
   YaspEventList__Validator(UnicodeComposer & erhArg) : erh(erhArg) {}
   void inspectTagStart(Tag const &);
   void inspectTagEnd(Tag const &);
private:
   UnicodeComposer & erh;
};

class YASP3_API YaspEventList {     // FIFO
public:
   YaspEventList(UnicodeComposer & erh);

   bool isEventWaiting() const;
   int count() const;
   void next(YaspEvent & evCurr);
   bool operator+=(YaspEvent const & ev);
   void setValidation(bool isToEnable);
   void reset();

private:
   class YASP3_API RepList : public TpList {
   public:
      TP_MakeSequence(RefdItemRep);
   };
   RepList  rlst;
   TpIxList tlst;
   bool isValidate;
   YaspEventList__Validator validator;
};

/* -- INLINES -- */
inline YaspEventList::YaspEventList(
   UnicodeComposer & erh
) :
   validator(erh), isValidate(false) {
}
inline bool YaspEventList::isEventWaiting() const {
   if (tlst.count()) return true; else return false;
}
inline int YaspEventList::count() const {
   return tlst.count();
}
inline bool YaspEventList::operator+=(YaspEvent const & ev) {
   rlst.insertLast((RefdItemRep const *)ev.inqData());
   tlst += ev.inqType();
   return true;
}
inline void YaspEventList::setValidation(bool isToEnable) {
   isValidate = isToEnable;
}
inline void YaspEventList::next(YaspEvent & evCurr) {
   if (tlst.count()) {
      evCurr = YaspEvent(rlst[0], (YaspType)tlst[0]);
      rlst.removeFirst();
      tlst.removeFirst();
      if (isValidate) evCurr.accept(validator);
   }else {
      evCurr = YaspEvent::Nil;
   }
}

/*--------------------------------------------------------class YaspStringizer-+
| Inspector to stringize events                                               |
+----------------------------------------------------------------------------*/
class YASP3_API YaspStringizer : public YaspEventInspector
{
public:
   YaspStringizer(
      Writer & uost,
      bool isFullAttlist = false
   );
   void inspectDtdStart(           Entity const &                );
   void inspectDclElement(         Element const &               );
   void inspectDclElementAttlist(  Element const &               );
   void inspectDclEntity(          Entity const &                );
   void inspectDclNotation(        Notation const &              );
   void inspectDclNotationAttlist( Notation const &              );
   void inspectTagStart(           Tag const &                   );
   void inspectTagEnd(             Tag const &                   );
   void inspectTagEmpty(           Tag const &                   );
   void inspectText(               TextData const &              );
   void inspectCdataText(          TextData const &              );
   void inspectSdataText(          TextData const &              );
   void inspectPiText(             TextData const &              );
   void inspectCommentText(        TextData const &              );
   void inspectEntityRef(          Entity const &                );
   void inspectEntityStart(        Entity const &                );
   void inspectEntityEnd(          Entity const &                );
   void inspectMarkedStart(        MarkedSectionData const &     );
   void inspectXmlPi(              XmlPiData const &             );

private:
   Writer & m_uost;
   bool m_isFullAttlist;
};

/* -- INLINES -- */
inline YaspStringizer::YaspStringizer(
   Writer & uost,
   bool isFullAttlist
) :
   m_uost(uost),
   m_isFullAttlist(isFullAttlist)
{}

#endif /* YSPEVENT_HEADER ===================================================*/
