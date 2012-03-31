/*
* $Id: rcdbind.cpp,v 1.1 2002-01-17 06:57:44 pgr Exp $
*
* Static values for the RE/RS flowgraph
*
* +--------+-----------------------------------------+
* |        |           E v e n t s                   |
* | states | RS     RE     STG    ETG    DATA  OTHERS|
* |        |                                         |
* |    0   |  5   |  2   |  0   |  1   |  1   |  0   |
* |    1   |  3   |  6   |  0   |  1   |  1   |  1   |
* |    2   |  5   |  6   |  0   |  1   |  1   |  0   |
* |    3   | 3+RE | 6+RE | 0+RE |  1   | 1+RE |  4   |
* |    4   |  3   |  6   | 0+RE |  1   | 1+RE |  4   |
* |    5   |  3   |  6   |  0   |  1   |  1   |  0   |
* |    6   |  3   | 6+RE | 0+RE |  1   | 1+RE |  4   |
* |--------+------+------+------+------+------+------|
* |    7   | 7+RE | 8+RE |  7   |  7   |  7   |  7   |
* |    8   |  7   | 8+RE |  7   |  7   |  7   |  7   |
* +--------------------------------------------------+
*
*/

/*---------+
| Includes |
+---------*/
#include "rcdbind.h"

// -------------------------- states ----------------------------------- Event
//   0       1       2       3       4       5       6       7       8

RecordBinder::FlowGraphNode const RecordBinder::fgRS[9] = {
   {5,0} , {3,0} , {5,0} , {3,1} , {3,0} , {3,0} , {3,0} , {7,1} , {7,0} //RS
};
RecordBinder::FlowGraphNode const RecordBinder::fgRE[9] = {
   {2,0} , {6,0} , {6,0} , {6,1} , {6,0} , {6,0} , {6,1} , {8,1} , {8,1} //RE
};
RecordBinder::FlowGraphNode const RecordBinder::fgSTG[9] = {
   {0,0} , {0,0} , {0,0} , {0,1} , {0,1} , {0,0} , {0,1} , {7,0} , {7,0} //STG
};
RecordBinder::FlowGraphNode const RecordBinder::fgETG[9] = {
   {1,0} , {1,0} , {1,0} , {1,0} , {1,0} , {1,0} , {1,0} , {7,0} , {7,0} //ETG
};
RecordBinder::FlowGraphNode const RecordBinder::fgDATA[9] = {
   {1,0} , {1,0} , {1,0} , {1,1} , {1,1} , {1,0} , {1,1} , {7,0} , {7,0} //DATA
};
RecordBinder::FlowGraphNode const RecordBinder::fgOTHER[9] = {
   {0,0} , {1,0} , {0,0} , {4,0} , {4,0} , {0,0} , {4,0} , {7,0} , {7,0} //OTH
};

/*============================================================================*/
