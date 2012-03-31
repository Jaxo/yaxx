/* $Id: Constants.h,v 1.57 2002-10-24 13:48:00 pgr Exp $ */

#ifndef COM_JAXO_YAXX_CONSTANTS_H_INCLUDED
#define COM_JAXO_YAXX_CONSTANTS_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "RexxString.h"
#include "../reslib/Keyword.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*--------------------------------------------------------- struct Constants -+
|                                                                             |
+----------------------------------------------------------------------------*/
struct Constants {
   static char const FILE_SEPARATOR;
   static char const PATH_SEPARATOR;
   static RexxString STR_COMMAND;
   static RexxString STR_DEFAULT_SHELL;
   static RexxString STR_EMPTY;
   static RexxString STR_ENGINEERING;
   static RexxString STR_ERROR;
   static RexxString STR_FAILURE;
   static RexxString STR_FUNCTION;
   static RexxString STR_HALT;
   static RexxString STR_LOSTDIGITS;
   static RexxString STR_NORMAL;
   static RexxString STR_NOTREADY;
   static RexxString STR_NOVALUE;
   static RexxString STR_OPERATING_SYSTEM;
   static RexxString STR_RC;
   static RexxString STR_READY;
   static RexxString STR_RESULT;
   static RexxString STR_SCIENTIFIC;
   static RexxString STR_SIGL;
   static RexxString STR_SUBROUTINE;
   static RexxString STR_SYNTAX;
   static RexxString STR_UNKNOWN;
   static RexxString STR_ANY_SHELL;

   static RexxString STR_STREAM_OPEN;
   static RexxString STR_STREAM_CLOSE;
   static RexxString STR_STREAM_FLUSH;
   static RexxString STR_STREAM_RESET;
   static RexxString STR_UNKNOWN_PATH;

};

/*------------------------------------------------------------ enum Property -+
| Property                                                                    |
+----------------------------------------------------------------------------*/
enum Property {
   PROP_ENVIRONMENT,         // address environment
   PROP_TRACE,               // trace option
   PROP_DIGITS,              // numeric digits
   PROP_FUZZ,                // fuzz digits
   PROP_FORM,                // numeric form
   PROP_VERSION,             // version string
   PROP_SET_SIGNAL,          // signal condition
   PROP_SET_SIGNAL_NAME,     // signal name cond
   PROP_UNSET_SIGNAL,        // signal condition
   PROP_SET_CALLON,          // callon condition
   PROP_SET_CALLON_NAME,     // callon name cond
   PROP_UNSET_CALLON,        // callon condition
   PROP_SOURCE
};

/*------------------------------------------------------------ enum CallType -+
| Call types                                                                  |
+----------------------------------------------------------------------------*/
enum CallType {
   CT_COMMAND,            // main routine
   CT_SUBROUTINE,
   CT_FUNCTION,
   CT_INTERPRET
};

/*-------------------------------------------------------------- enum OpCode -+
| Operation Codes                                                             |
+----------------------------------------------------------------------------*/
enum OpCode {
   I_CLAUSE,             // clause (used for tracing)
   I_LABEL,              // label clause (used for tracing)
   I_PUSH,               // push a RexxString
   I_PUSHLIT,            // push and trace the pushed RexxString p
   I_PUSHTMP,            // push a temporary RexxString
   I_POP,                // pop a RexxString
   I_COPY,               // COPY to previous
   I_COPY2TMP,           // COPY a value to temporary

   I_LOAD_VAR,           // load a variable
   I_CREATE_VAR,         // create and assign a var
   I_DROP_VAR,           // drop a variable directly
   I_DROP_REF,           // drop a variable indirectly
   I_SET_STEM,           // set the stem array
   I_LOAD_ARG,           // load argument
   I_IMM_VAL,            // Immediate numeric value
   I_IMM_STRING,         // Immediate string

   I_JMP_ALWAYS,         // Unconditional jump
   I_JMP_FALSE,          // Jump if false
   I_JMP_TRUE,           // Jump if true
   I_JMP_LBL,            // Jump to label
   I_JMP_VAL,            // Jump to expressed label
   I_NOP,                // Jump after me! (dummy instruction)
   I_CALL_ANY,           // Call a subroutine or a function
   I_CALL_EXT,           // As above, but bypass internals
   I_CALL_REF,           // Call by reference (YAXX extension)
   I_INTERPRET,          // interpret a string
   I_SYSTEM,             // execute a system command
   I_RETURN_EMPTY,       // return with no expression
   I_RETURN_RESULT,      // return with an evaluated expression
   I_EXIT_EMPTY,         // exit with no expression
   I_EXIT_RESULT,        // exit with an evaluated expression
   I_ERROR,              // run time error
   I_PROC,               // procedure subroutine or function (an error)

   I_PLUS,               // monadic arithmetic plus prefix
   I_MINUS,              // monadic arithmetic minus prefix
   I_NOT,                // monadic boolean negation
   I_AND,                // monadic boolean and
   I_OR,                 // monadic boolean or
   I_XOR,                // monadic boolean exclusive or
   I_ADD,                // dyadic add
   I_SUB,                // dyadic subtract
   I_MUL,                // dyadic multiplication
   I_DIV,                // dyadic division
   I_IDIV,               // dyadic integer division
   I_MOD,                // dyadic modulo
   I_POWER,              // dyadic power
   I_CONCAT_A,           // dyadic (abut two strings)
   I_CONCAT_B,           // dyadic (concat two strings with space)

   // this has to be kept grouped, and in this order.  don't ask.
   I_NORMAL_GT,          // dyadic comparison: normal greather than
   I_NORMAL_EQ,          // dyadic comparison: normal equal
   I_NORMAL_GE,          // dyadic comparison: normal greater ot equal
   I_NORMAL_LT,          // dyadic comparison: normal less than
   I_NORMAL_NE,          // dyadic comparison: normal not equal
   I_NORMAL_LE,          // dyadic comparison: normal less or equal

   // this has to be kept grouped, and in this order.  don't ask.
   I_STRICT_GT,          // dyadic comparison: strict greather than
   I_STRICT_EQ,          // dyadic comparison: strict equal
   I_STRICT_GE,          // dyadic comparison: strict greater ot equal
   I_STRICT_LT,          // dyadic comparison: strict less than
   I_STRICT_NE,          // dyadic comparison: strict not equal
   I_STRICT_LE,          // dyadic comparison: strict less or equal

   I_SAY,                // print on "cout"
   I_OPTIONS,            // options (not really implemented)
   I_GET_PROP,           // get property (NUMERIC etc..)
   I_SET_PROP,           // set property

   I_DO_CTL_INIT,        // Control variable is numeric
   I_DO_TO_INIT,         // TO is a number
   I_DO_BY_INIT,         // BY is a number
   I_DO_FOR_INIT,        // FOR is positive whole number
   I_DO_TO_BY_TEST,      // DO .. TO .. BY
   I_DO_TO_TEST,         // DO .. TO
   I_DO_FOR_TEST,        // DO .. FOR
   I_DO_BY_STEP,         // step the control variable by the BY value
   I_DO_STEP,            // step the control variable by 1

   I_TEMPLATE_START,     // start of template
   I_TARGET_VAR,         // parse into a variable target
   I_TARGET_DOT,         // parse into a placeholder target
   I_TRIG_SPACE,         // trigger a space
   I_TRIG_LIT,           // trigger a litteral
   I_TRIG_ABS,           // trigger absolute pos
   I_TRIG_PLUS_REL,      // trigger relative pos forward
   I_TRIG_MINUS_REL,     // trigger relative pos backward
   I_TEMPLATE_END,       // end of template

   I_STACK_QUEUE,        // enqueue
   I_STACK_PUSH,         // push
   I_STACK_PULL,         // pop
   I_STACK_LINEIN,       // push linein

   I_UPPER,              // to uppercase a lit

   I_EOF,                // End of file

   I_lastMN              // should be the last!
};

/*--------------------------------------------------------------- enum Token -+
| Tokens returned by the Tokenizer                                            |
+----------------------------------------------------------------------------*/
enum Token {
   TK_I_MASK                = 0xFF,

   TK_CONSTANT_SYMBOL        = 1 << 8,
   TK_NUMBER_CONSTANT_SYMBOL = TK_CONSTANT_SYMBOL + 0,
   TK_STRING_CONSTANT_SYMBOL = TK_CONSTANT_SYMBOL + 1,
   TK_OTHER_CONSTANT_SYMBOL  = TK_CONSTANT_SYMBOL + 2,

   TK_VARIABLE_SYMBOL        = 1 << 9,
   TK_SIMPLE_SYMBOL          = TK_VARIABLE_SYMBOL + 0,
   TK_COMPOUND_SYMBOL        = TK_VARIABLE_SYMBOL + 1,
   TK_STEM_SYMBOL            = TK_VARIABLE_SYMBOL + 2,

   TK_SYMBOL                 = TK_CONSTANT_SYMBOL | TK_VARIABLE_SYMBOL,

   TK_FUNCTION               = 1 << 10,
   TK_FUNCTION_LITERAL       = TK_FUNCTION + 1,

   TK_SYMBOL_OR_FUNCTION     = TK_SYMBOL | TK_FUNCTION,

   TK_OPGROUP_0              = 1 << 11,
   TK_OPGROUP_2              = 1 << 12,   // Comparison operators
   TK_OPGROUP_5              = 1 << 13,   // Multiplication and Division
   TK_OPERATOR               = TK_OPGROUP_0 | TK_OPGROUP_2 | TK_OPGROUP_5,

   TK_LEFTPAR                = TK_OPGROUP_0 + I_lastMN  + 0, // (
   TK_PLUS                   = TK_OPGROUP_0 + I_ADD,         // +
   TK_MINUS                  = TK_OPGROUP_0 + I_SUB,         // -
   TK_NOT                    = TK_OPGROUP_0 + I_NOT,         // \ or ^
   TK_RIGHTPAR               = TK_OPGROUP_0 + I_lastMN  + 1, // )
   TK_DOT                    = TK_OPGROUP_0 + I_lastMN  + 2, // .
   TK_POWER                  = TK_OPGROUP_0 + I_POWER,       // **
   TK_AND                    = TK_OPGROUP_0 + I_AND,         // &
   TK_OR                     = TK_OPGROUP_0 + I_OR,          // |
   TK_XOR                    = TK_OPGROUP_0 + I_XOR,         // &&
   TK_CONCAT                 = TK_OPGROUP_0 + I_CONCAT_A,    // ||
   TK_COMMA                  = TK_OPGROUP_0 + I_lastMN  + 3, // ,
   TK_SEMICOLON              = TK_OPGROUP_0 + I_lastMN  + 4, // ;

   TK_NORMAL_EQ              = TK_OPGROUP_2 + I_NORMAL_EQ,   // =
   TK_NORMAL_GE              = TK_OPGROUP_2 + I_NORMAL_GE,   // >=
   TK_NORMAL_GT              = TK_OPGROUP_2 + I_NORMAL_GT,   // >
   TK_NORMAL_LE              = TK_OPGROUP_2 + I_NORMAL_LE,   // <=
   TK_NORMAL_LT              = TK_OPGROUP_2 + I_NORMAL_LT,   // <
   TK_NORMAL_NE              = TK_OPGROUP_2 + I_NORMAL_NE,   // ^=
   TK_STRICT_EQ              = TK_OPGROUP_2 + I_STRICT_EQ,   // ==
   TK_STRICT_GE              = TK_OPGROUP_2 + I_STRICT_GE,   // >>=
   TK_STRICT_GT              = TK_OPGROUP_2 + I_STRICT_GT,   // >>
   TK_STRICT_LE              = TK_OPGROUP_2 + I_STRICT_LE,   // <<=
   TK_STRICT_LT              = TK_OPGROUP_2 + I_STRICT_LT,   // <<
   TK_STRICT_NE              = TK_OPGROUP_2 + I_STRICT_NE,   // ^==

   TK_MUL                    = TK_OPGROUP_5 + I_MUL,         // *
   TK_DIV                    = TK_OPGROUP_5 + I_DIV,         // /
   TK_MOD                    = TK_OPGROUP_5 + I_MOD,         // //
   TK_IDIV                   = TK_OPGROUP_5 + I_IDIV,        // %

   TK_KEYWORD                = 1 << 14,
   TK_TO                     = TK_KEYWORD   + _KWD__TO,
   TK_BY                     = TK_KEYWORD   + _KWD__BY,
   TK_FOR                    = TK_KEYWORD   + _KWD__FOR,
   TK_UNTIL                  = TK_KEYWORD   + _KWD__UNTIL,
   TK_WHILE                  = TK_KEYWORD   + _KWD__WHILE,
   TK_THEN                   = TK_KEYWORD   + _KWD__THEN,
   TK_WITH                   = TK_KEYWORD   + _KWD__WITH,
   TK_END                    = TK_KEYWORD   + _KWD__END,

   TK_OTHERS                 = 1 << 15,
   TK_LABEL                  = TK_OTHERS    + 1,
   TK_EOF                    = TK_OTHERS    + 2
};

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
