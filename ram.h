/*ram.h*/

/**
  * @brief Random access memory (RAM) for nuPython
  *
  * To execute Python code, the nuPython interpreter needs to store the
  * values of Python variables. The RAM functions serve this purpose.
  *
  * @note Prof. Joe Hummel
  * @note Northwestern University
  */

#pragma once

#include <stdbool.h>  // true, false


//
// Definition of random access memory (RAM)
//
enum RAM_VALUE_TYPES
{
  RAM_TYPE_INT = 0,
  RAM_TYPE_REAL,
  RAM_TYPE_STR,
  RAM_TYPE_PTR,
  RAM_TYPE_BOOLEAN,
  RAM_TYPE_NONE
};

struct RAM_VALUE
{
  //
  // What type of value is stored here?
  //
  int value_type;  // enum RAM_VALUE_TYPES

  //
  // the actual value when type != NONE:
  //
  union
  {
    int    i; // INT, PTR, BOOLEAN
    double d; // REAL
    char*  s; // STR 
  } types;
};

struct RAM_MAP
{
  char* varname;  // variable name
  int   cell;     // memory cell assigned to variable
};

struct RAM
{
  struct RAM_VALUE* cells;  // array of memory cells
  struct RAM_MAP*   map;    // ordered array to map vars to memory cells
  int size;                 // # of vars currently in memory
  int capacity;             // total # of cells available in memory
};


//
// Public functions:
//

/**
  * @brief ram_init: initialize memory unit
  *
  * Returns a pointer to a dynamically-allocated memory
  * for storing nuPython variables and their values. All
  * memory cells are initialized to the value None. You
  * take ownership of the returned memory and must call
  * ram_destroy() when you are done.
  *
  * @return pointer to struct denoting memory unit
  */
struct RAM* ram_init(void);

/**
  * @brief ram_destroy: frees memory associated with memory unit
  * 
  * Frees the dynamically-allocated memory associated with
  * the given memory. After the call returns, you cannot
  * use the memory.
  *
  * @return void
  */
void ram_destroy(struct RAM* memory);

/**
  * @brief ram_size: # of vars in memory
  *
  * Returns the # of variables currently stored in memory.
  *
  * @return # of vars in memory
  */
int ram_size(struct RAM* memory);

/**
  * @brief ram_capacity: # of cells available in memory
  *
  * Returns the # of cells currently available in memory.
  *
  * @return # of cells available in memory
  */
int ram_capacity(struct RAM* memory);

/**
  * @brief ram_get_addr: address of memory cell occupied by variable
  *
  * If the given variable (e.g. "x") has been written to 
  * memory, returns the address of this variable --- an integer
  * in the range 0..N-1 where N is the number of vars currently 
  * stored in memory. Returns -1 if no such variable exists 
  * in memory. 
  *
  * NOTE: a variable has to be written to memory before you can
  * get its address. Once a variable is written to memory, its
  * address never changes. 
  *
  * @param memory Pointer to struct denoting memory unit
  * @param varname variable name
  * @return address of variable or -1 if doesn't exist
  */
int ram_get_addr(struct RAM* memory, char* varname);

/**
  * @brief ram_read_cell_by_addr: returns value in memory cell at this address
  *
  * Given a memory address (an integer in the range 0..N-1), 
  * returns a COPY of the value contained in that memory cell.
  * Returns NULL if the address is not valid.
  * 
  * NOTE: this function allocates memory for the value that
  * is returned. The caller takes ownership of the copy and 
  * must eventually free this memory via ram_free_value().
  *
  * NOTE: a variable has to be written to memory before its
  * address becomes valid. Once a variable is written to memory,
  * its address never changes.
  *
  * @param memory Pointer to struct denoting memory unit
  * @param address memory cell address
  * @return pointer to struct containing value or NULL if doesn't exist
  */
struct RAM_VALUE* ram_read_cell_by_addr(struct RAM* memory, int address);

/**
  * @brief ram_read_cell_by_name: returns value in memory cell for this variable
  *
  * If the given variable (e.g. "x") has been written to 
  * memory, returns a COPY of the value contained in memory.
  * Returns NULL if no such name exists in memory.
  *
  * NOTE: this function allocates memory for the value that
  * is returned. The caller takes ownership of the copy and 
  * must eventually free this memory via ram_free_value().
  *
  * @param memory Pointer to struct denoting memory unit
  * @param varname variable name
  * @return pointer to struct containing value or NULL if doesn't exist
  */
struct RAM_VALUE* ram_read_cell_by_name(struct RAM* memory, char* varname);

/**
  * @brief ram_free_value: free value returned by read_cell() functions
  *
  * Frees the memory value returned by ram_read_cell_by_name and
  * ram_read_cell_by_addr.
  *
  * @param value Pointer to struct containing value
  * @return void
  */
void ram_free_value(struct RAM_VALUE* value);

/**
  * @brief ram_write_cell_by_addr: writes a value to memory cell at this address
  *
  * Writes the given value to the memory cell at the given 
  * address. If a value already exists at this address, that
  * value is overwritten by this new value. Returns true if 
  * the value was successfully written, false if not (which 
  * implies the memory address is invalid).
  *
  * NOTE: if the value being written is a string, it will
  * be duplicated and stored.
  * 
  * NOTE: a variable has to be written to memory before its
  * address becomes valid. Once a variable is written to memory,
  * its address never changes.
  *
  * @param memory Pointer to struct denoting memory unit
  * @param value value to be written to memory
  * @param address memory cell address
  * @return true if successful, false if not (invalid address)
  */
bool ram_write_cell_by_addr(struct RAM* memory, struct RAM_VALUE value, int address);

/**
  * ram_write_cell_by_name
  *
  * Writes the given value to a memory cell named by the given
  * variable. If a memory cell already exists with this name,
  * the existing value is overwritten by this new value. Returns
  * true since this operation always succeeds.
  *
  * NOTE: if the value being written is a string, it will
  * be duplicated and stored.
  *
  * NOTE: a variable has to be written to memory before its
  * address becomes valid. Once a variable is written to memory,
  * its address never changes.
  *
  * @param memory Pointer to struct denoting memory unit
  * @param value value to be written to memory
  * @param varname variable name
  * @return true (always successful)
  */
bool ram_write_cell_by_name(struct RAM* memory, struct RAM_VALUE value, char* varname);

/**
  * @brief ram_print: prints the contents of memory
  *
  * Prints the contents of RAM to the console, for debugging.
  * RAM is printed in alphabetical order by variable name.
  *
  * @param memory Pointer to struct denoting memory unit
  * @return void
  */
void ram_print(struct RAM* memory);

/**
  * @brief ram_print_map: prints the contents of memory map
  *
  * Prints contents of RAM map to the console, for debugging.
  *
  * @param memory Pointer to struct denoting memory unit
  * @return void
  */
void ram_print_map(struct RAM* memory);
