/************************************************************************
*
*  lap.h
   version 1.0 - 21 june 1996
   author  Roy Jonker, MagicLogic Optimization Inc.
   
   header file for LAP
*
**************************************************************************/

/*************** CONSTANTS  *******************/

  #define BIG 100000

/*************** TYPES      *******************/

  typedef int row;
  typedef int col;
  //typedef int cost;
  typedef float cost;

/*************** FUNCTIONS  *******************/

extern int lap(
	int dim, 
	cost **assigncost,
    int *rowsol, 
	int *colsol, 
	cost *u, 
	cost *v);

extern void checklap(
	int dim, 
	cost **assigncost,                 
	int *rowsol, 
	int *colsol, 
	cost *u, 
	cost *v);