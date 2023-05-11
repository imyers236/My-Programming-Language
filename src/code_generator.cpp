//----------------------------------------------------------------------
// FILE: code_generator.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: 
// DESC: 
//----------------------------------------------------------------------

#include <iostream>             // for debugging
#include "code_generator.h"

using namespace std;


// helper function to replace all occurrences of old string with new
void replace_all(string& s, const string& old_str, const string& new_str)
{
  while (s.find(old_str) != string::npos)
    s.replace(s.find(old_str), old_str.size(), new_str);
}


CodeGenerator::CodeGenerator(VM& vm)
  : vm(vm)
{
}


void CodeGenerator::visit(Program& p)
{
  for (auto& struct_def : p.struct_defs)
    struct_def.accept(*this);
  for (auto& fun_def : p.fun_defs)
    fun_def.accept(*this);
}


/**
 * The function generates code for a function definition and adds it to a virtual machine.
 * 
 * @param f The parameter `f` is a reference to a `FunDef` object, which represents a function
 * definition in the abstract syntax tree (AST) of a programming language. The `FunDef` object contains
 * information about the function name, parameters, and body statements.
 */
void CodeGenerator::visit(FunDef& f)
{
  string name = f.fun_name.lexeme();
  int param_size = f.params.size();
  VMFrameInfo frame {name, param_size};
  curr_frame = frame;
  var_table.push_environment();
  for(int i = 0; i < f.params.size(); i++)
  {
    curr_frame.instructions.push_back(VMInstr::STORE(i));
    var_table.add(f.params[i].var_name.lexeme());
  }
  for(auto s: f.stmts)
  {
    s->accept(*this);
  }
  if(curr_frame.instructions.empty() || curr_frame.instructions.back().opcode() != OpCode::RET)
  {
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
    curr_frame.instructions.push_back(VMInstr::RET());
  }
  vm.add(curr_frame);
  var_table.pop_environment();
  next_var_index = 0;

}


/**
 * The function stores a StructDef object in a map with the struct name as the key.
 * 
 * @param s The parameter `s` is a reference to an object of type `StructDef`.
 */
void CodeGenerator::visit(StructDef& s)
{
  struct_defs[s.struct_name.lexeme()] = s;
}


/**
 * This function generates virtual machine instructions for a return statement.
 * 
 * @param s The parameter "s" is a reference to a ReturnStmt object.
 */
void CodeGenerator::visit(ReturnStmt& s)
{
  s.expr.accept(*this);
  curr_frame.instructions.push_back(VMInstr::RET());
}

/**
 * This function generates a VM instruction to delete a struct object.
 * 
 * @param d The parameter "d" is a reference to a DeleteStructStmt object.
 */
void CodeGenerator::visit(DeleteStructStmt& d)
{
  d.expr.accept(*this);
  curr_frame.instructions.push_back(VMInstr::DELS());
}

/**
 * This function generates virtual machine instructions for deleting an array.
 * 
 * @param d The parameter "d" is a reference to a DeleteArrayStmt object.
 */
void CodeGenerator::visit(DeleteArrayStmt& d)
{
  d.expr.accept(*this);
  curr_frame.instructions.push_back(VMInstr::DELAR());
}


/**
 * This function generates code for a while loop.
 * 
 * @param s WhileStmt object that represents a while loop in the code.
 */
void CodeGenerator::visit(WhileStmt& s)
{
  int start = curr_frame.instructions.size();
  s.condition.accept(*this);
  int jmpf = curr_frame.instructions.size();
  curr_frame.instructions.push_back(VMInstr::JMPF(-1));
  var_table.push_environment();
  for(auto t: s.stmts)
  {
    t->accept(*this);
  }
  var_table.pop_environment();
  curr_frame.instructions.push_back(VMInstr::JMP(start));
  curr_frame.instructions.push_back(VMInstr::NOP());
  int nop = curr_frame.instructions.size() - 1;
  curr_frame.instructions.at(jmpf).set_operand(nop);
}


/**
 * This function generates code for a for loop statement.
 * 
 * @param s The parameter "s" is a reference to a ForStmt object, which represents a for loop statement
 * in the abstract syntax tree of a programming language. The code generator is visiting this statement
 * and generating corresponding virtual machine instructions for it.
 */
void CodeGenerator::visit(ForStmt& s)
{
  var_table.push_environment();
  s.var_decl.accept(*this);
  int start = curr_frame.instructions.size();
  s.condition.accept(*this);
  int jmpf = curr_frame.instructions.size();
  curr_frame.instructions.push_back(VMInstr::JMPF(-1));
  var_table.push_environment();
  for(auto t: s.stmts)
  {
    t->accept(*this);
  }
  var_table.pop_environment();
  s.assign_stmt.accept(*this);
  var_table.pop_environment();
  curr_frame.instructions.push_back(VMInstr::JMP(start));
  curr_frame.instructions.push_back(VMInstr::NOP());
  int nop = curr_frame.instructions.size() - 1;
  curr_frame.instructions.at(jmpf).set_operand(nop);
}


/**
 * This function generates code for an if statement with optional else-if and else clauses.
 * 
 * @param s The parameter `s` is a reference to an `IfStmt` object, which represents an if statement in
 * the abstract syntax tree of a program.
 */
void CodeGenerator::visit(IfStmt& s)
{
  vector<int> jmps;
  vector<int> jmpf;
  s.if_part.condition.accept(*this);
  jmpf.push_back(curr_frame.instructions.size());
  curr_frame.instructions.push_back(VMInstr::JMPF(-1));
  var_table.push_environment();
  for(auto& t : s.if_part.stmts)
  {
    t->accept(*this);
  }
  var_table.pop_environment();
  jmps.push_back(curr_frame.instructions.size());
  curr_frame.instructions.push_back(VMInstr::JMP(-1));
  jmpf.push_back(curr_frame.instructions.size());
  curr_frame.instructions.push_back(VMInstr::NOP());
  curr_frame.instructions.at(jmpf[0]).set_operand(jmpf[1]);
  for(auto e : s.else_ifs)
  {
    int else_counter = 2;
    e.condition.accept(*this);
    jmpf.push_back(curr_frame.instructions.size());
    curr_frame.instructions.push_back(VMInstr::JMPF(-1));
    var_table.push_environment();
    for(auto& t : e.stmts)
    {
      t->accept(*this);
    }
    var_table.pop_environment();
    jmps.push_back(curr_frame.instructions.size());
    curr_frame.instructions.push_back(VMInstr::JMP(-1));
    jmpf.push_back(curr_frame.instructions.size());
    curr_frame.instructions.push_back(VMInstr::NOP());
    curr_frame.instructions.at(jmpf[else_counter]).set_operand(jmpf[else_counter + 1]);
  }
  for(auto p : s.else_stmts)
  {
    p->accept(*this);
  }
  int end = curr_frame.instructions.size();
  curr_frame.instructions.push_back(VMInstr::NOP());
  for(auto t: jmps)
  {
    curr_frame.instructions.at(t).set_operand(end);
  }
  
}


/**
 * This function visits a variable declaration statement, adds the variable to a table, and generates a
 * store instruction for the current frame.
 * 
 * @param s VarDeclStmt reference, which is the statement node in the abstract syntax tree representing
 * a variable declaration statement.
 */
void CodeGenerator::visit(VarDeclStmt& s)
{
  s.expr.accept(*this);
  var_table.add(s.var_def.var_name.lexeme());
  curr_frame.instructions.push_back(VMInstr::STORE(var_table.get(s.var_def.var_name.lexeme())));
}

void CodeGenerator::visit(AssignStmt& s)
{
  curr_frame.instructions.push_back(VMInstr::LOAD(var_table.get(s.lvalue[0].var_name.lexeme())));
  for(int i = 0; i < s.lvalue.size(); i++)
  {
    VarRef v = s.lvalue[i];
    string name = v.var_name.lexeme();
    if(i != 0)
    {
      curr_frame.instructions.push_back(VMInstr::GETF(name));
    }
    if(v.array_expr.has_value())
    {
      v.array_expr->accept(*this);
      curr_frame.instructions.push_back(VMInstr::GETI());
    }
  }
  curr_frame.instructions.pop_back();
  s.expr.accept(*this);
  if(s.lvalue.size() > 1 && s.lvalue.back().array_expr == nullopt)
  {
    curr_frame.instructions.push_back(VMInstr::SETF(s.lvalue.back().var_name.lexeme()));
  }
  else if(s.lvalue.back().array_expr != nullopt)
  {
    curr_frame.instructions.push_back(VMInstr::SETI());
  }
  else
  {
    curr_frame.instructions.push_back(VMInstr::STORE(var_table.get(s.lvalue.back().var_name.lexeme())));
  }
}


/**
 * The function visits a CallExpr node and generates corresponding virtual machine instructions based
 * on the function name.
 * 
 * @param e The parameter "e" is a reference to a CallExpr object, which represents a function call
 * expression in the abstract syntax tree of a programming language.
 */
void CodeGenerator::visit(CallExpr& e)
{
  string fun_name = e.fun_name.lexeme();
  for(int i = 0; i < e.args.size(); i++)
  {
    e.args[i].accept(*this);
  }
  if(fun_name == "print")
  {
    curr_frame.instructions.push_back(VMInstr::WRITE());
  }
  else if(fun_name == "to_string")
  {
    curr_frame.instructions.push_back(VMInstr::TOSTR());
  }
  else if(fun_name == "to_int")
  {
    curr_frame.instructions.push_back(VMInstr::TOINT());
  }
  else if(fun_name == "to_double")
  {
    curr_frame.instructions.push_back(VMInstr::TODBL());
  }
  else if(fun_name == "concat")
  {
    curr_frame.instructions.push_back(VMInstr::CONCAT());
  }
  else if(fun_name == "input")
  {
    curr_frame.instructions.push_back(VMInstr::READ());
  }
  else if(fun_name == "length")
  {
      curr_frame.instructions.push_back(VMInstr::SLEN());
  }
  else if(fun_name == "array_length")
  {
      curr_frame.instructions.push_back(VMInstr::ALEN());
  }
  else if(fun_name == "get")
  {
    curr_frame.instructions.push_back(VMInstr::GETC());
  }
  else
  {
    curr_frame.instructions.push_back(VMInstr::CALL(fun_name));
  }

}


/**
 * This function generates virtual machine instructions for an expression, based on its operators and
 * operands.
 * 
 * @param e The parameter "e" is an object of the class "Expr", which represents an expression in the
 * programming language being compiled. The method "visit" is a member function of the "CodeGenerator"
 * class, and it is called to generate code for the given expression. The method traverses the
 * expression
 */
void CodeGenerator::visit(Expr& e)
{
  e.first->accept(*this);
  if(e.op.has_value())
  {
    e.rest->accept(*this);
    if(e.op.value().lexeme() == "+")
    {
      curr_frame.instructions.push_back(VMInstr::ADD());
    }
    else if(e.op.value().lexeme() == "-")
    {
      curr_frame.instructions.push_back(VMInstr::SUB());
    }
    else if(e.op.value().lexeme() == "*")
    {
      curr_frame.instructions.push_back(VMInstr::MUL());
    }
    else if(e.op.value().lexeme() == "/")
    {
      curr_frame.instructions.push_back(VMInstr::DIV());
    }
    else if(e.op.value().lexeme() == "<")
    {
      curr_frame.instructions.push_back(VMInstr::CMPLT());
    }
    else if(e.op.value().lexeme() == "<=")
    {
      curr_frame.instructions.push_back(VMInstr::CMPLE());
    }
    else if(e.op.value().lexeme() == ">")
    {
      curr_frame.instructions.push_back(VMInstr::CMPGT());
    }
    else if(e.op.value().lexeme() == ">=")
    {
      curr_frame.instructions.push_back(VMInstr::CMPGE());
    }
    else if(e.op.value().lexeme() == "==")
    {
      curr_frame.instructions.push_back(VMInstr::CMPEQ());
    }
    else if(e.op.value().lexeme() == "!=")
    {
      curr_frame.instructions.push_back(VMInstr::CMPNE());
    }
    else if(e.op.value().lexeme() == "and")
    {
      curr_frame.instructions.push_back(VMInstr::AND());
    }
    else if(e.op.value().lexeme() == "or")
    {
      curr_frame.instructions.push_back(VMInstr::OR());
    }
  }
  if(e.negated == true)
  {
   curr_frame.instructions.push_back(VMInstr::NOT());
  }
}


/**
 * The function visits a SimpleTerm and calls the accept function on its rvalue.
 * 
 * @param t The parameter "t" is a reference to an object of class SimpleTerm.
 */
void CodeGenerator::visit(SimpleTerm& t)
{
  t.rvalue->accept(*this);
}
 

/**
 * The function visits a complex term and accepts its expression.
 * 
 * @param t The parameter "t" is a reference to an object of the class "ComplexTerm".
 */
void CodeGenerator::visit(ComplexTerm& t)
{
  t.expr.accept(*this);
}


/**
 * The function generates virtual machine instructions for pushing different types of values onto the
 * stack based on the type of the given simple r-value.
 * 
 * @param v The parameter `v` is a reference to a `SimpleRValue` object, which represents a simple
 * right-hand side value in an abstract syntax tree (AST) node. The `visit` method is a member function
 * of a `CodeGenerator` class, which generates virtual machine (VM) instructions
 */
void CodeGenerator::visit(SimpleRValue& v)
{
  if(v.value.type() == TokenType::INT_VAL)
  {
    int val = stoi(v.value.lexeme());
    curr_frame.instructions.push_back(VMInstr::PUSH(val));
  }
  else if(v.value.type() == TokenType::DOUBLE_VAL)
  {
    double val = stod(v.value.lexeme());
    curr_frame.instructions.push_back(VMInstr::PUSH(val));
  }
  else if(v.value.type() == TokenType::STRING_VAL)
  {
    string val = v.value.lexeme();
    replace_all(val, "\\n", "\n");
    replace_all(val, "\\t", "\t");
    curr_frame.instructions.push_back(VMInstr::PUSH(val));
  }
  else if(v.value.type() == TokenType::BOOL_VAL)
  {
    if(v.value.lexeme() == "true")
      curr_frame.instructions.push_back(VMInstr::PUSH(true));
    else
      curr_frame.instructions.push_back(VMInstr::PUSH(false));
  }
  else if(v.value.type() == TokenType::CHAR_VAL)
  {
    string val = v.value.lexeme();
    replace_all(val, "\\n", "\n");
    replace_all(val, "\\t", "\t");
    curr_frame.instructions.push_back(VMInstr::PUSH(val));
  }
  else if (v.value.type() == TokenType::NULL_VAL) 
  {
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
  } 
}


/**
 * The function generates code for creating a new object or array in the virtual machine.
 * 
 * @param v The parameter "v" is a reference to a NewRValue object, which represents the creation of a
 * new value in the code being generated. The NewRValue object may have an optional array expression
 * and a type associated with it. The code generator is responsible for generating the appropriate
 * instructions to create the
 */
void CodeGenerator::visit(NewRValue& v)
{
  if(v.array_expr.has_value())
  {
    v.array_expr->accept(*this);
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
    curr_frame.instructions.push_back(VMInstr::ALLOCA());
  }
  else
  {
    curr_frame.instructions.push_back(VMInstr::ALLOCS());
    for(auto& field : struct_defs[v.type.lexeme()].fields)
    {
      curr_frame.instructions.push_back(VMInstr::DUP());
      curr_frame.instructions.push_back(VMInstr::ADDF(field.var_name.lexeme()));
      curr_frame.instructions.push_back(VMInstr::DUP());
      curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
      curr_frame.instructions.push_back(VMInstr::SETF(field.var_name.lexeme()));
    }
  }
}


/**
 * This function generates virtual machine instructions for loading a variable's value and accessing
 * its fields and array elements.
 * 
 * @param v VarRValue object that represents a variable reference in the AST (Abstract Syntax Tree).
 */
void CodeGenerator::visit(VarRValue& v)
{
  curr_frame.instructions.push_back(VMInstr::LOAD(var_table.get(v.path[0].var_name.lexeme())));
  for(int i = 0; i < v.path.size(); i++)
  {
    VarRef r = v.path[i];
    string name = r.var_name.lexeme();
    if(i != 0)
    {
      curr_frame.instructions.push_back(VMInstr::GETF(name));
    }
    if(r.array_expr.has_value())
    {
      r.array_expr->accept(*this);
      curr_frame.instructions.push_back(VMInstr::GETI());
    }
  }
}
    

