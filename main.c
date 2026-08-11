/*main.c*/

/** 
  * @brief Main program to scan, parse, and execute nuPython programs.
  *
  * @note Nnedi Nwosu
  *
  * @note Starter code: Prof. Joe Hummel
  * @note Northwestern University
  */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>   // strcspn

#include "token.h"    // token defs
#include "scanner.h" 
#include "parser.h"

#include "programgraph.h"  // for program graph data structures
#include "ram.h"           // for RAMH data structures and functions
#include "execute.h"         // for program execution functions




/**
  * @brief main()
  *
  * usage: program.exe [filename.py]
  *
  * If a filename is given, the file is opened and serves as
  * input to the program. If a filename is not given, then 
  * input is taken from the keyboard until $ is input.
  *
  * @return 0 => success
  */
int main(int argc, char* argv[])
{
  FILE* input = NULL;
  bool  keyboardInput = false;

  //
  // where is the input coming from?
  //
  if (argc < 2) {
    //
    // no args, just the program name:
    //
    input = stdin;
    keyboardInput = true;
  }
  else {
    //
    // assume 2nd arg is a nuPython file:
    //
    char* filename = argv[1];

    input = fopen(filename, "r");

    if (input == NULL) // unable to open:
    {
      printf("**ERROR: unable to open input file '%s' for input.\n", filename);
      return 0;
    }

    keyboardInput = false;
  }

  if (keyboardInput)  // prompt the user if appropriate:
  {
    printf("nuPython input (enter $ when you're done)>\n");
  }

  //
  // call parser to check program syntax:
  //
  struct TokenQueue* tokens = parser_parse(input);

  if (tokens == NULL)
  {
    // 
    // program has a syntax error, error msg already output:
    //
    printf("**parsing failed...\n");
  }
  else
  {
    printf("**parsing successful, valid syntax\n");

    printf("**building program graph...\n");


    //build the program graph
    struct STMT* program = programgraph_build(tokens);
    if (program == NULL){  
      return 0;
    }

    //print the program graph
    //programgraph_print(program);



    //create memory for execution
    printf("**executing...\n");
    struct RAM* memory =ram_init();

    //execute the program
    execute(program, memory);
    printf("**done\n");
    
    //print the contents of memory after execution
    ram_print(memory);

    //free the memeory used for the program graph and memory data structures
    ram_destroy(memory);
    programgraph_destroy(program);
    tokenqueue_destroy(tokens);
  }

  //
  // done:
  //
  if (!keyboardInput)
    fclose(input);

  return 0;
}
