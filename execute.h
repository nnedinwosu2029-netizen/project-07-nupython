/*execute.h*/

/**
  * @brief Executes nuPython program, given as a Program Graph.
  *
  * @note Prof. Joe Hummel
  * @note Northwestern University
  */

#pragma once

#include "programgraph.h"
#include "ram.h"

//
// Public functions:
//

/**
  * @brief executes a nuPython program
  *
  * Given a nuPython program graph and a memory, 
  * executes the statements in the program graph.
  * If a semantic error occurs (e.g. type error),
  * an error message is output, execution stops,
  * and the function returns.
  *
  * @param program pointer to first stmt in program graph
  * @param memory pointer to memory unit
  * @return void
  */
void execute(struct STMT* program, struct RAM* memory);
