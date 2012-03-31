/*
* $Id: rcdbind.h,v 1.5 2002-04-14 23:26:04 jlatone Exp $
*
*  Record Binder (rule at 7.6.1)
*
* This class describes the record boundaries conditions of 7.6.1.
*
* In contents, the following events:  must be told to the binder by calling:
*    Record start:                       `tellRS'
*    Record end:                         `tellRE'
*    data (not RE, nor RS):              `tellData'
*    start of a proper sub-element:      `tellStartProper'
*    end of a proper sub-element:        `tellEndProper'
*    start of an included sub-element:   `tellStartInclusion'
*    end of an included sub-element:     `tellEndInclusion'
*    other markup declarations:          `tellOtherMarkup'
*
* Three of these functions:
*      `tellRE',
*      `tellStartProper',
*      `tellData'
* might return true, meaning a significant record end has been found.
*
* In theory, each time a new element is started, either proper or included,
* the current FlowGraph should be stacked and a new one started.
* This reinforces the rule for each element being treated as an atom.
* According to this, the graph can be reduced to 5 states and 4 events:
*
*   - RS      : Record Start
*   - RE      : Record End
*   - DATA    : Data or Proper subelement
*   - OTHER   : Included subelement, Markup Declaration, PI
*
*   Current|   New State = f(Events)   |
*    State |                           |
*          |  RS  |  RE  | DATA |OTHER |
*   -------+------+------+------+------+
*      0   |  2   |  2   |  1   |  0   |
*      1   |  2   |  3   |  1   |  1   |
*      2   |  2   |  3   |  1   |  0   |
*      3   |  3   | 3+RE | 1+RE |  4   |
*      4   |  3   |  3   | 1+RE |  4   |
*
* State 0 and 2 reflect "RE due to markup" conditions.
* State 3 and 4 are "RE pending" conditions: the RE will be generated
* or not according to the last RE rule (rule b).
* State 0 and 4 are to discard the RE following an included subelement.
* State 0 is also the initial state (graph reduction).
* State 1 is when a DATA or proper subelement is included.
*
* In practice, entering a new proper element doesn't require a stack operation:
* the RE (if any) will be generated before the proper subelement is entered
* so the state "popped" when the proper subelement ends is always the same.
* Therefore, the start and ends of a proper subelement (STG and ETG) can be
* added to the list of events.  However, for included subelement, stacking
* the element's current state is mandatory.
*
* Additions:
*
* - An RS without preceding RE should be seen as RE+RS (UNIX file)
*   This added states 5 and 6.
* - for HTML, create a "disabled" state (7/8) to handle verbatim elements
*
* The final graph is as shown below.
*
*   Current|  New State = f(Events)                  |
*    State |                                         |
*          |  RS  |  RE  | STG  | ETG  | DATA |OTHER |
*   -------+------+------+------+------+------+------+
*      0   |  5   |  2   |  0   |  1   |  1   |  0   |
*      1   |  3   |  6   |  0   |  1   |  1   |  1   |
*      2   |  5   |  6   |  0   |  1   |  1   |  0   |
*      3   | 3+RE | 6+RE | 0+RE |  1   | 1+RE |  4   |
*      4   |  3   |  6   | 0+RE |  1   | 1+RE |  4   |
*      5   |  3   |  6   |  0   |  1   |  1   |  0   |
*      6   |  3   | 6+RE | 0+RE |  1   | 1+RE |  4   |
*   -------+------+------+------+------+------+------|
*      7   | 7+RE | 8+RE |  7   |  7   |  7   |  7   |
*      8   |  7   | 8+RE |  7   |  7   |  7   |  7   |
*/

#if !defined RCDBIND_HEADER && defined __cplusplus
#define RCDBIND_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include <assert.h>
#include <string.h>

class YASP3_API RecordBinder {
public:
   RecordBinder();
   RecordBinder& operator=(RecordBinder const& source);
   RecordBinder(RecordBinder const& source);
   void reset();

   bool tellRS();
   bool tellRE();
   bool tellStartProper();
   void tellEndProper();
   bool tellData();
   void tellOtherMarkup();
   void tellStartInclusion();
   void tellEndInclusion();

   void disable();
   void enable();

   struct FlowGraphNode {
      char new_state;
      bool re_generated;
   };
private:
   static FlowGraphNode const fgRS[9];
   static FlowGraphNode const fgRE[9];
   static FlowGraphNode const fgSTG[9];
   static FlowGraphNode const fgETG[9];
   static FlowGraphNode const fgDATA[9];
   static FlowGraphNode const fgOTHER[9];
   char *pState;       // current
   char state[100];    // hope I got enough (shouldn't be greater than TAGLVL)
};

/* -- INLINES -- */
inline void RecordBinder::reset() {
   pState = state;
   *pState = 0;
}
inline RecordBinder::RecordBinder() {
   reset();
}
inline RecordBinder::RecordBinder(RecordBinder const& source) {
   int i = source.pState - source.state;
   memcpy(state, source.state, (i+1) * sizeof state[0]);
   pState = state + i;
}
inline RecordBinder& RecordBinder::operator=(RecordBinder const& source) {
   int i = source.pState - source.state;
   memcpy(state, source.state, (i+1) * sizeof state[0]);
   pState = state + i;
   return * this;
}
inline bool RecordBinder::tellRS() {
   FlowGraphNode old_node = fgRS[*pState];
   *pState = old_node.new_state;
   return old_node.re_generated;
}
inline bool RecordBinder::tellRE() {
   FlowGraphNode old_node = fgRE[*pState];
   *pState = old_node.new_state;
   return old_node.re_generated;
}
inline bool RecordBinder::tellStartProper() {
   FlowGraphNode old_node = fgSTG[*pState];
   *pState = old_node.new_state;
   return old_node.re_generated;
}
inline void RecordBinder::tellEndProper() {
   *pState = fgETG[*pState].new_state;
}
inline bool RecordBinder::tellData() {
   FlowGraphNode old_node = fgDATA[*pState];
   *pState = old_node.new_state;
   return old_node.re_generated;
}
inline void RecordBinder::tellOtherMarkup() {
   *pState = fgOTHER[*pState].new_state;
}
inline void RecordBinder::tellStartInclusion() {
   assert  (pState < state + (sizeof state - 1));
   tellOtherMarkup();
   *(++pState) = 0;
}
inline void RecordBinder::tellEndInclusion() {
   --pState;
}
inline void RecordBinder::disable() {
   *pState = 7;
}
inline void RecordBinder::enable() {
   *pState = 1;
}
#endif /* RCDBIND.HEADER =====================================================*/
