/*execute.c*/


/**
  * @brief Executes nuPython program, given as a Program Graph.
  *
  * This file contains fucntions that execute a nuPython program,
  * represented as a program graph. It supports assignments,
  * functions calls, pass statements, relational and arithmetic
  * operators, if statements, and while loops. 
  *
  * @note Nnedi Nwosu
  *
  * @note Starter code: Prof. Joe Hummel
  * @note Northwestern University
  */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>
#include <assert.h>

#include "programgraph.h"
#include "ram.h"
#include "execute.h"
#include "math.h"


//
// Public functions:
//

static bool execute_stmts(struct STMT* program, struct STMT* stop, struct RAM* memory);

/** 
* @brief Duplicates a string into dynamically allocated memory
*
* This function creates and returns a copy of a given string 
* in heap memory. The caller is responsible for freeing the 
* memory allocated.
* 
* @param other pointer to the string to duplicate
*
* @return the newly allocated copy of the string
**/
static char* dupString (char* other){

  //allocate memory for the copy
  size_t len = strlen(other) + 1;
  char* copy = (char*)malloc(len*sizeof(char));

  //copy other into copy
  strcpy(copy, other);

  //return the copy
  return copy;
}

/** 
* @brief Return the value of the uniary expression
*
* This function returns the value of a uniary expression, 
* which may be a literal, an identifier, or a function call. 
* Returns true if the expression is executed successfully, 
* and false if there is a semantic error.
*
* @param stmt pointer to the current statment
* @param memory pointer to the memory data structure
* @param lhs pointer to the unary expression
* @param result pointer to the RAM_VALUE strcut that stores the result of the executed expression
*
* @return returns true if the expression is executed successfully, false if there is a semantic error
**/
static bool execute_uniary_expr(struct STMT* stmt, struct RAM* memory, struct UNARY_EXPR* lhs, struct RAM_VALUE* result){

  //store the value of the uniary expression based on the element type

  if (lhs->element->element_type == ELEMENT_INT_LITERAL){ //integer literal
      result->value_type = RAM_TYPE_INT;
      result->types.i = atoi(lhs->element->element_value);
    }

    if (lhs->element->element_type == ELEMENT_REAL_LITERAL){ //real literal
      result->value_type = RAM_TYPE_REAL;
      result->types.d = atof(lhs->element->element_value);
    }

    if (lhs->element->element_type == ELEMENT_STR_LITERAL){ //string literal
      result->value_type = RAM_TYPE_STR;
      result->types.s = dupString(lhs->element->element_value);
    }

    if (lhs->element->element_type == ELEMENT_TRUE){
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = 1; //true is represented as 1 in memory
    }

    if (lhs->element->element_type == ELEMENT_FALSE){
      result->value_type = RAM_TYPE_BOOLEAN;
      result->types.i = 0; //false is represented as 0 in memory
    }

    if (lhs->element->element_type == ELEMENT_IDENTIFIER){ //identifier

      struct RAM_VALUE* value = ram_read_cell_by_name(memory, lhs->element->element_value);

      if (value == NULL){
        printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", lhs->element->element_value, stmt->line);
        return false;
      }

      //store the result based on the value type
      else {

        result->value_type = value->value_type;

        if (value->value_type == RAM_TYPE_INT){
          result->types.i = value->types.i;
        }

        if (value->value_type == RAM_TYPE_REAL){
          result->types.d = value->types.d;
        }

        if (value->value_type == RAM_TYPE_STR){
          result->types.s = value->types.s;
        }

        if (value->value_type == RAM_TYPE_BOOLEAN){
          result->types.i = value->types.i;
        }
        
    }
  }

  return true;
}

/** 
* @brief Checks if a string consists of all zero characters
*
* This function checks if a given string consists of all zero characters. It returns
* true if the string consists of all zero characters, and false if not. It also returns 
* false if the string is NULL or empty.
*
* @param s pointer to the string to check
*
* @return true if the string consists of all zero characters, false if not
**/
static bool is_all_zeros (char* s){

  //ignore negative sign
  if (*s == '-'){ 
    s++;
  }

  bool has_zero = false; //flag to track if char is 0
  while (*s == '0' || *s == '.'){ 
    if (*s == '0'){
      has_zero = true;
    }
    s++; //advance to the next char
  }

  return has_zero && *s == '\0'; //return true if all chars in string are 0
}



/** 
* @brief Executes a binary expression between two integer operands 
*
* This function executes a binary expression between two integer 
* operands. It supports the +, -, *, /, %, and ** arithmatic 
* operators, and the ==, !=, <, <=, >, >= relational operators. 
* Returns true if the expression is executed successfully, and 
* false if there is a semantic error.
*
* @param stmt pointer to the current statment
* @param rhs_expr pointer to the binary expression
* @param executed_expr pointer to the value of the executed expression
* @param lhs_value the value of the left-hand side operand
* @param rhs_value the value of the right-hand side operand
*
* @return true if the expression is executed successfully, false if there is a semantic error
**/
static bool execute_int(struct STMT* stmt, struct EXPR* rhs_expr, struct RAM_VALUE* executed_expr,  struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value){

  //assign value type of the executed expression to int
  executed_expr->value_type = RAM_TYPE_INT;

  //perform the appropriate arithmatic operation based on the operator type
  if (rhs_expr->operator_type == OPERATOR_PLUS){
    executed_expr->types.i = lhs_value.types.i + rhs_value.types.i;
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_MINUS){
    executed_expr->types.i = lhs_value.types.i - rhs_value.types.i;
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_ASTERISK){
    executed_expr->types.i = lhs_value.types.i * rhs_value.types.i;
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_POWER){
    executed_expr->types.i = (int)round(pow(lhs_value.types.i, rhs_value.types.i));
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_MOD){
    if (rhs_value.types.i == 0){
      printf("**SEMANTIC ERROR: modulus by 0 (line %d)\n", stmt->line);
      return false;
    }
    else{
      executed_expr->types.i = lhs_value.types.i % rhs_value.types.i;
      return true;
    }
  }

  else if (rhs_expr->operator_type == OPERATOR_DIV){
    if (rhs_value.types.i == 0){
      printf("**SEMANTIC ERROR: division by 0 (line %d)\n", stmt->line);
      return false;
    }
    else{
      executed_expr->types.i = lhs_value.types.i / rhs_value.types.i;
      return true;
    }
  }
  
  //change value type of the executed expression to boolean for relational operators
  executed_expr->value_type = RAM_TYPE_BOOLEAN;

  //perform the appropriate relational operation based on the operator type
  if (rhs_expr->operator_type == OPERATOR_EQUAL){
    executed_expr->types.i = (lhs_value.types.i == rhs_value.types.i);
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_NOT_EQUAL){
    executed_expr->types.i = (lhs_value.types.i != rhs_value.types.i);
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_LT){
    executed_expr->types.i = (lhs_value.types.i < rhs_value.types.i);
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_LTE){
    executed_expr->types.i = (lhs_value.types.i <= rhs_value.types.i);
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_GT){
    executed_expr->types.i = (lhs_value.types.i > rhs_value.types.i);
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_GTE){
    executed_expr->types.i = (lhs_value.types.i >= rhs_value.types.i);
    return true;
  }

  else{
    printf("**SEMANTIC ERROR: unsupported operator (line %d)\n", stmt->line);
    return false;
  }

  return false;
}

/** 
* @brief Executes a binary expression between two real operands 
* or one real and one integer operand
*
* This function executes a binary expression between two real 
* operands or one real and one integer operand. It supports the 
* +, -, *, /, %, and ** arithmatic operators, and the ==, !=, <, 
* <=, >, >= relational operators. Returns true if the expression 
* is executed successfully, and false if there is a semantic error.
*
* @param stmt pointer to the current statment
* @param rhs_expr pointer to the binary expression
* @param executed_expr pointer to the value of the executed expression
* @param lhs_value the value of the left-hand side operand
* @param rhs_value the value of the right-hand side operand
*
* @return true if the expression is executed successfully, false if there is a semantic error
**/
static bool execute_real(struct STMT* stmt, struct EXPR* rhs_expr, struct RAM_VALUE* executed_expr,  struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value){

  //assign value type of the executed expression to real
  executed_expr->value_type = RAM_TYPE_REAL;

  //perform the appropriate arithmatic operation based on the operator type
  if (rhs_expr->operator_type == OPERATOR_PLUS){
    executed_expr->types.d = lhs_value.types.d + rhs_value.types.d;
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_MINUS){
    executed_expr->types.d = lhs_value.types.d - rhs_value.types.d;
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_ASTERISK){
    executed_expr->types.d = lhs_value.types.d * rhs_value.types.d;
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_POWER){
    executed_expr->types.d = pow(lhs_value.types.d, rhs_value.types.d);
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_MOD){
    if (rhs_value.types.d == 0){
      printf("**SEMANTIC ERROR: modulus by 0 (line %d)\n", stmt->line);
      return false;
    }
    else{
      executed_expr->types.d = fmod(lhs_value.types.d, rhs_value.types.d);
      return true;
    }
  }

  else if (rhs_expr->operator_type == OPERATOR_DIV){
    if (rhs_value.types.d == 0){
      printf("**SEMANTIC ERROR: division by 0 (line %d)\n", stmt->line);
      return false;
    }
    else{
      executed_expr->types.d = lhs_value.types.d / rhs_value.types.d;
      return true;
    }
  }

  //change value type of the executed expression to boolean for relational operators
  executed_expr->value_type = RAM_TYPE_BOOLEAN;

  //perform the appropriate relational operation based on the operator type
  if (rhs_expr->operator_type == OPERATOR_EQUAL){
    executed_expr->types.i = (lhs_value.types.d == rhs_value.types.d);
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_NOT_EQUAL){
    executed_expr->types.i = (lhs_value.types.d != rhs_value.types.d);
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_LT){
    executed_expr->types.i = (lhs_value.types.d < rhs_value.types.d);
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_LTE){
    executed_expr->types.i = (lhs_value.types.d <= rhs_value.types.d);
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_GT){
    executed_expr->types.i = (lhs_value.types.d > rhs_value.types.d);
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_GTE){
    executed_expr->types.i = (lhs_value.types.d >= rhs_value.types.d);
    return true;
  }

  else{
    printf("**SEMANTIC ERROR: unsupported operator (line %d)\n", stmt->line);
    return false;
  } 

  return false;
}

/** 
* @brief Executes a binary expression between two string operands
*
* This function executes a binary expression between two string operands. 
* It supports the + operator for string concatenation, and the ==, !=, <,
*  <=, >, >= operators for string comparison. Returns true if the expression 
* is executed successfully, and false if there is a semantic error.
*
* @param stmt pointer to the current statment
* @param rhs_expr pointer to the binary expression
* @param executed_expr pointer to the value of the executed expression
* @param lhs_value the value of the left-hand side operand
* @param rhs_value the value of the right-hand side operand
*
* @return true if the expression is executed successfully, false if there is a semantic error
**/
static bool execute_str(struct STMT* stmt, struct EXPR* rhs_expr, struct RAM_VALUE* executed_expr, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value){

  

  //perform concatenation if the operator is +
  if (rhs_expr->operator_type == OPERATOR_PLUS){
    executed_expr->value_type = RAM_TYPE_STR;
    char* new_str = malloc(strlen(lhs_value.types.s) + strlen(rhs_value.types.s) + 1);
    strcpy(new_str, lhs_value.types.s);
    strcat(new_str, rhs_value.types.s);
    executed_expr->types.s = new_str;
    return true;
  }

  //change value type of the executed expression to boolean for relational operators
  executed_expr->value_type = RAM_TYPE_BOOLEAN;

  //perform string comparison based on the operator type using strcmp
  if (rhs_expr->operator_type == OPERATOR_EQUAL){
    int result = strcmp(lhs_value.types.s, rhs_value.types.s);
    if (result == 0){
      executed_expr->types.i = 1;
    }
    else{
      executed_expr->types.i = 0;
    }
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_NOT_EQUAL){
    int result = strcmp(lhs_value.types.s, rhs_value.types.s);
    if (result != 0){
      executed_expr->types.i = 1;
    }
    else{
      executed_expr->types.i = 0;
    }
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_LT){
    int result = strcmp(lhs_value.types.s, rhs_value.types.s);
    if (result < 0){
      executed_expr->types.i = 1;
    }
    else{
      executed_expr->types.i = 0;
    }
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_LTE){
    int result = strcmp(lhs_value.types.s, rhs_value.types.s);
    if (result <= 0){
      executed_expr->types.i = 1;
    }
    else{
      executed_expr->types.i = 0;
    }
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_GT){
    int result = strcmp(lhs_value.types.s, rhs_value.types.s);
    if (result > 0){
      executed_expr->types.i = 1;
    }
    else{
      executed_expr->types.i = 0;
    }
    return true;
  }

  else if (rhs_expr->operator_type == OPERATOR_GTE){
    int result = strcmp(lhs_value.types.s, rhs_value.types.s);
    if (result >= 0){
      executed_expr->types.i = 1;
    }
    else{
      executed_expr->types.i = 0;
    }
    return true;
  }

  else{
    return false;
  }
}

/** 
* @brief Return the value of the LHS or RHS pointer
*
* This function returns the value of the left-hand side 
* or right-hand side pointer, and returns false if there 
* is a semantic error (e.g. type error).
*
* @param stmt pointer to the current statment
* @param memory pointer to the memory data structure
* @param expr pointer to the unary expression containing the pointer
* @param expr_value pointer to the value of the unary expression
*
* @return the value of the right-hand/left-hand side expression
**/
static bool retrieve_value(struct STMT* stmt, struct RAM* memory, struct UNARY_EXPR* expr, struct RAM_VALUE* expr_value){

  //check if the value is a string literal
  if (expr->element->element_type == ELEMENT_INT_LITERAL){
    expr_value->value_type = RAM_TYPE_INT;
    expr_value->types.i = atoi(expr->element->element_value); //retrieve and store value
    return true;
  }

  //check if the value is a string literal
  if (expr->element->element_type == ELEMENT_REAL_LITERAL){
    expr_value->value_type = RAM_TYPE_REAL;
    expr_value->types.d = atof(expr->element->element_value); //retrieve and store value
    return true;
  }

  //check if the value is a string literal
  if (expr->element->element_type == ELEMENT_STR_LITERAL){
    expr_value->value_type = RAM_TYPE_STR;
    expr_value->types.s = dupString(expr->element->element_value);//retrieve and store value
    return true;
  }

  //check if the value is a boolean
  if (expr->element->element_type == ELEMENT_TRUE){
    expr_value->value_type = RAM_TYPE_BOOLEAN;
    expr_value->types.i = 1;//retrieve and store value
    return true;
  }

  if (expr->element->element_type == ELEMENT_FALSE){
    expr_value->value_type = RAM_TYPE_BOOLEAN;
    expr_value->types.i = 0;//retrieve and store value
    return true;
  }

  //check if the value is an identifier
  if (expr->element->element_type == ELEMENT_IDENTIFIER){

    struct RAM_VALUE* value = ram_read_cell_by_name(memory, expr->element->element_value);

    if (value == NULL){
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", expr->element->element_value, stmt->line);
      return false;
    }

    //retrieve and store value
      if (value->value_type == RAM_TYPE_INT){
          expr_value->value_type = RAM_TYPE_INT;
          expr_value->types.i = value->types.i;
          return true;
      }

      if (value->value_type == RAM_TYPE_REAL){
          expr_value->value_type = RAM_TYPE_REAL;
          expr_value->types.d = value->types.d;
          return true;
      }

      if (value->value_type == RAM_TYPE_STR){
          expr_value->value_type = RAM_TYPE_STR;
          expr_value->types.s = value->types.s;
          return true;
      }

      if (value->value_type == RAM_TYPE_BOOLEAN){
          expr_value->value_type = RAM_TYPE_BOOLEAN;
          expr_value->types.i = value->types.i;
          return true;
      }

  }

  printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", expr->element->element_value, stmt->line);
  return false;
}


/** 
* @brief Return the value of the uniary or binary expression
*
* This function executes a uniary or binary expression. Returns 
* the value of the expression if it is executed successfully. It 
* returns false if there is a semantic error.
*
* @param stmt pointer to the current statment
* @param memory pointer to the memory data structure
* @param rhs_expr pointer to the uniary or binary expression
* @param executed_expr pointer to the RAM_VALUE struct that stores the value of the executed expression
*
* @return true if the expression is executed successfully, false if there is a semantic error (e.g. type error)
**/
static bool execute_expr(struct STMT* stmt, struct RAM* memory, struct EXPR* rhs_expr, struct RAM_VALUE* executed_expr){

  //execute uniary expression
  if (!rhs_expr->isBinaryExpr){ 
    return execute_uniary_expr(stmt, memory, rhs_expr->lhs, executed_expr);
  }

  //else, execute binary expression

  //intialize pointer values
  struct RAM_VALUE lhs_value;
  struct RAM_VALUE rhs_value;

  struct UNARY_EXPR* lhs = rhs_expr->lhs; //grabs the left hand side of the right-hand expression

  bool success = retrieve_value(stmt, memory, lhs, &lhs_value); //check if the lhs value is valid

  if (!success){
    return false;
  }

  struct UNARY_EXPR* rhs = rhs_expr->rhs; //grabs the right hand side of the right-hand expression
  success = retrieve_value(stmt, memory, rhs, &rhs_value); //check if the rhs value is valid
  if (!success){
    return false;
  }

  //execute the appropriate binary expression based on the value types of the operands
  if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT){ //int + int
    success = execute_int(stmt, rhs_expr, executed_expr, lhs_value, rhs_value);
    if (!success){
      return false;
    }
  }

  else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL){ //real + real
    success = execute_real(stmt, rhs_expr, executed_expr, lhs_value, rhs_value);
    if (!success){
      return false;
    }
  }

  else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_INT){ //real + int
    rhs_value.types.d = (double)rhs_value.types.i;
    success = execute_real(stmt, rhs_expr, executed_expr, lhs_value, rhs_value);
    if (!success){
      return false;
    }
  }

  else if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_REAL){ //int + real
    lhs_value.types.d = (double)lhs_value.types.i;
    success = execute_real(stmt, rhs_expr, executed_expr, lhs_value, rhs_value);
    if (!success){
      return false;
    }
  }

  else if (lhs_value.value_type == RAM_TYPE_STR && rhs_value.value_type == RAM_TYPE_STR){ //string + string
    success = execute_str(stmt, rhs_expr, executed_expr, lhs_value, rhs_value);
    if (!success){
      return false;
    }
  }

  else{
    printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
    return false;
  }

  return true;
}


/** 
* @brief Executes an if statement
*
* This function executes an if statement. It returns true 
* if the statement executes successfully, false if not.
*
* @param stmt pointer to the current statment
* @param memory pointer to the memory data structure
*
* @return true if the current statement executes successfully, false if not (e.g. semantic error)
**/
static bool execute_if(struct STMT* stmt, struct RAM* memory){

  //evaluate the condition expression
  struct EXPR* condition_expr = stmt->types.if_then_else->condition;
  struct RAM_VALUE executed_condition;

  bool success = execute_expr(stmt, memory, condition_expr, &executed_condition);

  
  if (!success){
    return false;
  }

  if (executed_condition.value_type != RAM_TYPE_INT && executed_condition.value_type != RAM_TYPE_BOOLEAN){
      printf("**SEMANTIC ERROR: invalid condition type (line %d)\n", stmt->line);
      return false;
  }

  //execute the true path if the condition is true, otherwise execute the false path if it exists
  else {

    if (executed_condition.types.i != 0){

      success = execute_stmts(stmt->types.if_then_else->true_path, stmt->types.if_then_else->next_stmt, memory);
      if (!success){
        return false;
      }
    }

    else if (stmt->types.if_then_else->false_path != NULL ){
      struct STMT* false_path = stmt->types.if_then_else->false_path;

      if (false_path->stmt_type == STMT_IF_THEN_ELSE){
        success = execute_if(false_path, memory);
      }
      else{
        success = execute_stmts(false_path, stmt->types.if_then_else->next_stmt, memory);
      }
      if (!success){
        return false;
      }
    }
    return true;
}
}

/** 
* @brief Executes an while loop
*
* This function executes a while loops. It returns true 
* if the loop executes successfully, false if not.
*
* @param stmt pointer to the current statment
* @param memory pointer to the memory data structure
*
* @return true if the current statement executes successfully, false if not (e.g. semantic error)
**/
static bool execute_while(struct STMT* stmt, struct RAM* memory){

  //grabs the condition expression of the while loop
  struct EXPR* condition_expr = stmt->types.while_loop->condition;

  //evaluate the condition expression and execute the loop body while the condition is true
  while(true){

  //evaluate the condition expression
  struct RAM_VALUE executed_condition;
  if (!execute_expr(stmt, memory, condition_expr, &executed_condition)){
    return false;
  }

  bool isTrue = false; //flag to track if condition is true
  if (executed_condition.value_type == RAM_TYPE_INT || executed_condition.value_type == RAM_TYPE_BOOLEAN){
    if (executed_condition.types.i != 0){
      isTrue = true;
    }
  }
  else{
    printf("**SEMANTIC ERROR: invalid condition type (line %d)\n", stmt->line);
    return false;
  }

  if (isTrue){
    //execute the loop body if the condition is true, otherwise break out of the loop
    if (!execute_stmts(stmt->types.while_loop->loop_body, stmt->types.while_loop->next_stmt, memory)){
      return false;
    }
  }
  else{
    break;
  }
}
return true;
}




/** 
* @brief Evaluates the right-hand side of an assignment statement. 
*
* This function evaluates the right-hand side of a function call or 
* assignment statement. It stores the value of the expression in a 
* RAM_VALUE struct if the expression is evaluated successfully, and 
* returns false if there is a semantic error (e.g. type error).
*
* @param stmt pointer to the current statment
* @param memory pointer to the memory data structure
* @param result pointer to the RAM_VALUE taht stores the result
*
* @return true if the expression is evaluated successfully, false if not
**/
static bool evaluate_rhs(struct STMT* stmt, struct RAM* memory, struct RAM_VALUE* result){

    //check if the right-hand side is a function call
    if (stmt->types.assignment->rhs->value_type == VALUE_FUNCTION_CALL){

      //grab the function name and parameter
      char* func_name = stmt->types.assignment->rhs->types.function_call->function_name;
      struct ELEMENT* param = stmt->types.assignment->rhs->types.function_call->parameter;

      //execute the appropriate function based on the function name and parameter
      if (strcmp(func_name, "input") == 0){ //input() function

        printf("%s", param->element_value);

        //retreive value
        char line[512];
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\r\n")] = '\0';

        //store value
        result->value_type = RAM_TYPE_STR;
        result->types.s = dupString(line);    
      }

      else if (strcmp(func_name, "int") == 0){ //int() function
        //retreive and convert value


        struct RAM_VALUE* value = ram_read_cell_by_name(memory, param->element_value);
        int converted = atoi(value->types.s);
        if(converted == 0 && is_all_zeros(value->types.s) == false){
          printf("**SEMANTIC ERROR: conversion failed for int() (line %d)\n", stmt->line);
          return false;
        }
        //store value
        result->value_type = RAM_TYPE_INT;
        result->types.i = converted;
      }

      else if (strcmp(func_name, "float") == 0){ //float() function
        //retreive and convert value
        struct RAM_VALUE* value = ram_read_cell_by_name(memory, param->element_value); 
        double converted = atof(value->types.s);
        if(converted == 0 && is_all_zeros(value->types.s) == false){
          printf("**SEMANTIC ERROR: conversion failed for float() (line %d)\n", stmt->line);
          return false;
        }
        //store value
        result->value_type = RAM_TYPE_REAL;
        result->types.d = converted;

      }
    }

    //else, the right-hand side is an expression
    else{
      struct EXPR* rhs_expr = stmt->types.assignment->rhs->types.expr; //grabs the right hand side of the assignment statement
      return execute_expr(stmt, memory, rhs_expr, result);
    }

  return true;
}



/** 
* @brief Return true if the assignment statement executes successfully
*
* This function executes an assignment statement. It returns true 
* if the assignment executes successfully, false if not.
*
* @param stmt pointer to the current statment
* @param memory pointer to the memory data structure
*
* @return true if the current statement executes successfully, false if not (e.g. semantic error)
**/
static bool execute_assignment(struct STMT* stmt, struct RAM* memory){

//initialize variables
struct RAM_VALUE result = {0};
bool success;

//check if the right-hand side is a function call or an expression and evaluate accordingly
if (stmt->types.assignment->rhs->value_type == VALUE_FUNCTION_CALL){
  success = evaluate_rhs(stmt, memory, &result);
}

else{
  struct EXPR* rhs_expr = stmt->types.assignment->rhs->types.expr;
  success = execute_expr(stmt, memory, rhs_expr, &result);
}

if (!success){
  return false;
}

//check if the left-hand side is a pointer dereference
if (stmt->types.assignment->isPtrDeref){ 
  char* var_name = stmt->types.assignment->var_name;
  struct RAM_VALUE* value = ram_read_cell_by_name(memory, var_name);

  //check if the value exists
  if (value == NULL){
    printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
    return false;
  }

  //check if the value is an integer
  if (value->value_type != RAM_TYPE_INT){
    printf("**SEMANTIC ERROR: invalid address type for pointer assignment (line %d)\n", stmt->line);
    return false;
  } 

  int addr_num = value->types.i;

  //store the calculated value into memory
  success = ram_write_cell_by_addr(memory, result, addr_num);
  if (!success){
    printf("**SEMANTIC ERROR: invalid memory address for pointer assignment (line %d)\n", stmt->line);
    return false;
  }
  return true;
}

//else, the left-hand side is a normal variable assignment
else{ 
//store the calculated value into memory
  ram_write_cell_by_name(memory, result, stmt->types.assignment->var_name);
  return true;
  }
}


/** 
* @brief Return true if the function call executes successfully
*
* This function executes a function call. It returns true 
* if the function call executes successfully, false if not.
*
* @param stmt pointer to the current statment
* @param memory pointer to the memory data structure
*
* @return true if the current statement executes successfully, false if not (e.g. semantic error)
**/
static bool execute_function_call(struct STMT* stmt, struct RAM* memory){


  //if print() . . .
  if (strcmp(stmt->types.function_call->function_name, "print") == 0 && stmt->types.function_call->parameter == NULL){
    printf("\n");
    return true;
  }

  //if print() with a parameter . . .
  else if (strcmp(stmt->types.function_call->function_name, "print") == 0 && stmt->types.function_call->parameter != NULL){
   
    
    if (stmt->types.function_call->parameter->element_type == ELEMENT_STR_LITERAL){
      printf("%s\n", stmt->types.function_call->parameter->element_value);
      return true;
    }


    else if (stmt->types.function_call->parameter->element_type == ELEMENT_INT_LITERAL){
      printf("%d\n", atoi(stmt->types.function_call->parameter->element_value));
      return true;
    }


    else if (stmt->types.function_call->parameter->element_type == ELEMENT_REAL_LITERAL){
      printf("%lf\n", atof(stmt->types.function_call->parameter->element_value));
      return true;
    }

    else if (stmt->types.function_call->parameter->element_type == ELEMENT_TRUE){
      printf("True\n");
      return true;
    }


    else if (stmt->types.function_call->parameter->element_type == ELEMENT_FALSE){
      printf("False\n");
      return true;
    }


    else if (stmt->types.function_call->parameter->element_type == ELEMENT_IDENTIFIER){

      struct RAM_VALUE* value = ram_read_cell_by_name(memory, stmt->types.function_call->parameter->element_value);

      if (value == NULL){
        printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", stmt->types.function_call->parameter->element_value, stmt->line);
        return false;
      }

      else {

        if (value->value_type == RAM_TYPE_INT){
          printf("%i\n", value->types.i);
          return true;
        }

        if (value->value_type == RAM_TYPE_REAL){
          printf("%lf\n", value->types.d);
          return true;
        }

        if (value->value_type == RAM_TYPE_STR){
          printf("%s\n", value->types.s);
          return true;
        }

        if (value->value_type == RAM_TYPE_BOOLEAN){
          if (value->types.i == 0){
            printf("False\n");
            return true;
          }
          else {
            printf("True\n");
            return true;
          }
        }

      }
   
    }
  }

  //if function_name is anything other than "print", . . . 
    printf("**SEMANTIC ERROR: unknown function (line %d)\n", stmt->line);
    return false;

}

/**
  * @brief executes a nuPython program, returning true or false.
  *
  * This function is a helper function for execute() that executes 
  * the statements in a nuPython program graph. Returns true if 
  * execution is successful, and false if a semantic error occurs.
  * If a semantic error occurs, an error message is output and execution stops.
  *
  * @param program pointer to first stmt in program graph
  * @param memory pointer to memory unit
  * @return true if execution is successful, false if a semantic error occurs
  */
static bool execute_stmts(struct STMT* program,  struct STMT* stop, struct RAM* memory)
{

  struct STMT* stmt = program;

  //traverse and execute the program statements 
  while (stmt != NULL && stmt != stop){ 
    bool success = true;
    struct STMT* next_stmt = NULL; //var to store the next statement to execute

    if (stmt->stmt_type == STMT_ASSIGNMENT){  //assignment statement
      success = execute_assignment(stmt, memory);
      next_stmt = stmt->types.assignment->next_stmt; //advance the pointer
    }

    else if (stmt->stmt_type == STMT_FUNCTION_CALL){ //function call
      success = execute_function_call(stmt, memory);
      next_stmt = stmt->types.function_call->next_stmt; //advance the pointer
    }


    else if (stmt->stmt_type == STMT_IF_THEN_ELSE){ //if statement
      success = execute_if(stmt, memory);
      next_stmt = stmt->types.if_then_else->next_stmt; //advance the pointer
    }


    else if (stmt->stmt_type == STMT_WHILE_LOOP){ //while loop
      success = execute_while(stmt, memory);
      next_stmt = stmt->types.while_loop->next_stmt; //advance the pointer
    }


  else if (stmt->stmt_type == STMT_PASS){ //pass
      assert(stmt->stmt_type == STMT_PASS); 
      next_stmt = stmt->types.pass->next_stmt; //advance the pointer
    }

    if (!success){
      return false;
    }

    stmt = next_stmt; //advance to the next statement

  }
  
  return true;
}

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
void execute(struct STMT* program, struct RAM* memory)
{
  execute_stmts(program, NULL, memory);
}
