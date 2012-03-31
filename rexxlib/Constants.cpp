/* $Id: Constants.cpp,v 1.21 2011-08-31 07:25:20 pgr Exp $ */

#include "Constants.h"
#include "../reslib/Keyword.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

// #ifdef ANDROID
// #include <jni.h>
// jint JNI_OnLoad(JNIEnv * env, void *)
// {
//    jclass clazz = env->FindClass("android/os/Build#VERSION");
//    jint sdk = env->GetStaticIntField(
//       clazz, env->GetStaticFieldID(clazz, SDK_INT, "I")
//    );
//    switch (sdk) {
//    case  1: codeName = "BASE_1_0"; break;
//    case  2: codeName = "BASE_1_1"; break;
//    case  3: codeName = "CUPCAKE_1_5"; break;
//    case  4: codeName = "DONUT_1_6"; break;
//    case  5: codeName = "ECLAIR_2_0"; break;
//    case  6: codeName = "ECLAIR_2_0_1"; break;
//    case  7: codeName = "ECLAIR_2_1"; break;
//    case  8: codeName = "FROYO_2_2"; break;
//    case  9: codeName = "GINGERBREAD_2_3"; break;
//    case 10: codeName = "GINGERBREAD_2_3_3"; break;
//    case 11: codeName = "HONEYCOMB_3_0"; break;
//    case 12: codeName = "HONEYCOMB_3_1"; break;
//    case 13: codeName = "HONEYCOMB_3_2"; break;
//    default: codeName = "?"; break;
//    }
//    jstring release = env->GetStaticObjectField(
//       clazz, env->GetStaticFieldID(clazz, RELEASE, "Ljava/lang/String;")
//    );
//    return JNI_VERSION_1_1;
// #endif

// Operating system specifics
#if defined(_WIN32)
char const Constants::FILE_SEPARATOR = '\\';
char const Constants::PATH_SEPARATOR = ';';
RexxString Constants::STR_OPERATING_SYSTEM("WIN32");
RexxString Constants::STR_DEFAULT_SHELL("DOS");

#elif defined __MWERKS__
char const Constants::FILE_SEPARATOR = '/';
char const Constants::PATH_SEPARATOR = ':';
RexxString Constants::STR_OPERATING_SYSTEM("PalmOS");
RexxString Constants::STR_DEFAULT_SHELL("NONE");

#elif defined ANDROID
char const Constants::FILE_SEPARATOR = '/';
char const Constants::PATH_SEPARATOR = ':';
RexxString Constants::STR_OPERATING_SYSTEM("ANDROID");
RexxString Constants::STR_DEFAULT_SHELL("SH");
#else
char const Constants::FILE_SEPARATOR = '/';
char const Constants::PATH_SEPARATOR = ':';
RexxString Constants::STR_OPERATING_SYSTEM("UNIX");
RexxString Constants::STR_DEFAULT_SHELL("CSH");
#endif

RexxString Constants::STR_COMMAND(getKeyword(_KWD__COMMAND));
RexxString Constants::STR_EMPTY("");
RexxString Constants::STR_ENGINEERING(getKeyword(_KWD__ENGINEERING));
RexxString Constants::STR_ERROR(getKeyword(_KWD__ERROR));
RexxString Constants::STR_FAILURE(getKeyword(_KWD__FAILURE));
RexxString Constants::STR_FUNCTION(getKeyword(_KWD__FUNCTION));
RexxString Constants::STR_HALT(getKeyword(_KWD__HALT));
RexxString Constants::STR_LOSTDIGITS(getKeyword(_KWD__LOSTDIGITS));
RexxString Constants::STR_NORMAL(getKeyword(_KWD__NORMAL));
RexxString Constants::STR_NOTREADY(getKeyword(_KWD__NOTREADY));
RexxString Constants::STR_NOVALUE(getKeyword(_KWD__NOVALUE));
RexxString Constants::STR_RC(getKeyword(_KWD__RC));
RexxString Constants::STR_READY(getKeyword(_KWD__READY));
RexxString Constants::STR_RESULT(getKeyword(_KWD__RESULT));
RexxString Constants::STR_SCIENTIFIC(getKeyword(_KWD__SCIENTIFIC));
RexxString Constants::STR_SIGL(getKeyword(_KWD__SIGL));
RexxString Constants::STR_SUBROUTINE(getKeyword(_KWD__SUBROUTINE));
RexxString Constants::STR_SYNTAX(getKeyword(_KWD__SYNTAX));
RexxString Constants::STR_UNKNOWN("UNKNOWN");
RexxString Constants::STR_ANY_SHELL("SYSTEM");
RexxString Constants::STR_STREAM_OPEN("OPEN");
RexxString Constants::STR_STREAM_CLOSE("CLOSE");
RexxString Constants::STR_STREAM_FLUSH("FLUSH");
RexxString Constants::STR_STREAM_RESET("RESET");
RexxString Constants::STR_UNKNOWN_PATH("[UNKNOWN]");

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
