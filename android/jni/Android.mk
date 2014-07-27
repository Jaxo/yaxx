# $Id: Android.mk,v 1.6 2013-08-03 13:21:54 pgr Exp $

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog -lz
LOCAL_MODULE    := reslib
LOCAL_CFLAGS += -DCOM_JAXO_YAXX_DENY_XML
LOCAL_SRC_FILES := \
   ../../reslib/BuiltInName.cpp \
   ../../reslib/DayMonth.cpp \
   ../../reslib/HtmlDtd.cpp \
   ../../reslib/Keyword.cpp \
   ../../reslib/MsgTemplate.cpp
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog -lz
LOCAL_MODULE := decnblib
LOCAL_SRC_FILES := \
   ../../decnblib/DecNumber.cpp \
   ../../decnblib/DecRexx.cpp
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog -lz
LOCAL_MODULE    := toolslib
LOCAL_SHARED_LIBRARIES := reslib
LOCAL_SRC_FILES := \
   ../../toolslib/arglist.cpp \
   ../../toolslib/BinarySearch.cpp \
   ../../toolslib/ByteString.cpp \
   ../../toolslib/ConsoleSchemeHandler.cpp \
   ../../toolslib/Encoder.cpp \
   ../../toolslib/Escaper.cpp \
   ../../toolslib/FileStreamBuf.cpp \
   ../../toolslib/HttpSchemeHandler.cpp \
   ../../toolslib/LinkedList.cpp \
   ../../toolslib/mbstring.cpp \
   ../../toolslib/MemStream.cpp \
   ../../toolslib/miscutil.cpp \
   ../../toolslib/Reader.cpp \
   ../../toolslib/RegisteredURI.cpp \
   ../../toolslib/RefdItem.cpp \
   ../../toolslib/RefdKey.cpp \
   ../../toolslib/RWBuffer.cpp \
   ../../toolslib/sort.cpp \
   ../../toolslib/StdFileStream.cpp \
   ../../toolslib/StringBuffer.cpp \
   ../../toolslib/SystemContext.cpp \
   ../../toolslib/tpbhash.cpp \
   ../../toolslib/tpblist.cpp \
   ../../toolslib/tplist.cpp \
   ../../toolslib/tplistix.cpp \
   ../../toolslib/tplistwh.cpp \
   ../../toolslib/tpset.cpp \
   ../../toolslib/uccompos.cpp \
   ../../toolslib/ucformat.cpp \
   ../../toolslib/ucstring.cpp \
   ../../toolslib/URI.cpp \
   ../../toolslib/Writer.cpp \
   ../../toolslib/encoding/ecm.cpp \
   ../../toolslib/encoding/ecmc.cpp \
   ../../toolslib/encoding/ecmcp850.cpp \
   ../../toolslib/encoding/ecmeuc.cpp \
   ../../toolslib/encoding/ecmrom8.cpp \
   ../../toolslib/encoding/ecmsjis.cpp \
   ../../toolslib/encoding/ecmucs2.cpp \
   ../../toolslib/encoding/ecmucs2r.cpp \
   ../../toolslib/encoding/ecmutf8.cpp
include $(BUILD_SHARED_LIBRARY)

# include $(CLEAR_VARS)
# LOCAL_LDLIBS := -llog -lz
# LOCAL_MODULE := yasp3lib
# LOCAL_SHARED_LIBRARIES := toolslib
# LOCAL_SRC_FILES := \
#    ../../yasp3lib/syntax/charset.cpp \
#    ../../yasp3lib/syntax/dlmbuild.cpp \
#    ../../yasp3lib/syntax/dlmfind.cpp \
#    ../../yasp3lib/syntax/dlmlist.cpp \
#    ../../yasp3lib/syntax/features.cpp \
#    ../../yasp3lib/syntax/quantity.cpp \
#    ../../yasp3lib/syntax/rsvdname.cpp \
#    ../../yasp3lib/syntax/sdcl.cpp \
#    ../../yasp3lib/parser/rcdbind.cpp \
#    ../../yasp3lib/parser/ysperror.cpp \
#    ../../yasp3lib/parser/yspevent.cpp \
#    ../../yasp3lib/parser/yspgrab.cpp \
#    ../../yasp3lib/parser/yspinput.cpp \
#    ../../yasp3lib/parser/yspmain.cpp \
#    ../../yasp3lib/parser/yspparse.cpp \
#    ../../yasp3lib/parser/yspsdcl.cpp \
#    ../../yasp3lib/parser/ysptag.cpp \
#    ../../yasp3lib/parser/CachedDtd.cpp \
#    ../../yasp3lib/model/hitbits.cpp \
#    ../../yasp3lib/model/mdllctr.cpp \
#    ../../yasp3lib/model/mdltkn.cpp \
#    ../../yasp3lib/model/mdltree.cpp \
#    ../../yasp3lib/entity/entity.cpp \
#    ../../yasp3lib/entity/entmgr.cpp \
#    ../../yasp3lib/entity/entsolve.cpp \
#    ../../yasp3lib/entity/entstrea.cpp \
#    ../../yasp3lib/entity/externid.cpp \
#    ../../yasp3lib/element/attrib.cpp \
#    ../../yasp3lib/element/element.cpp \
#    ../../yasp3lib/element/elmmgr.cpp
# include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog -lz
LOCAL_MODULE := rexxlib
LOCAL_CFLAGS += -DCOM_JAXO_YAXX_DENY_XML
$(eval $(shell csh $(LOCAL_PATH)/../../version LOCAL_CFLAGS))
LOCAL_SHARED_LIBRARIES := toolslib reslib decnblib
LOCAL_SRC_FILES := \
   ../../rexxlib/Arguments.cpp \
   ../../rexxlib/BatchCompiler.cpp \
   ../../rexxlib/BuiltIn.cpp \
   ../../rexxlib/CharConverter.cpp \
   ../../rexxlib/Clauses.cpp \
   ../../rexxlib/CodeBuffer.cpp \
   ../../rexxlib/Compiler.cpp \
   ../../rexxlib/Constants.cpp \
   ../../rexxlib/DataQueueMgr.cpp \
   ../../rexxlib/Exception.cpp \
   ../../rexxlib/HashMap.cpp \
   ../../rexxlib/Interpreter.cpp \
   ../../rexxlib/IOMgr.cpp \
   ../../rexxlib/Label.cpp \
   ../../rexxlib/OnTimeCompiler.cpp \
   ../../rexxlib/Pool.cpp \
   ../../rexxlib/Rexx.cpp \
   ../../rexxlib/RexxString.cpp \
   ../../rexxlib/Routines.cpp \
   ../../rexxlib/Symbol.cpp \
   ../../rexxlib/TimeClock.cpp \
   ../../rexxlib/Tokenizer.cpp \
   ../../rexxlib/Tracer.cpp \
   ../../rexxlib/Variable.cpp \
   ../../rexxlib/VariableHandler.cpp \
   ../../rexxlib/XmlIOMgr.cpp
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog -lz
LOCAL_MODULE := androidlib
LOCAL_CFLAGS += -DCOM_JAXO_YAXX_DENY_XML
LOCAL_SHARED_LIBRARIES := toolslib reslib decnblib rexxlib
LOCAL_SRC_FILES := \
   ../../androidlib/JRexx.cpp \
   ../../androidlib/JConsole.cpp \
   ../../androidlib/SpeakerStream.cpp
include $(BUILD_SHARED_LIBRARY)
