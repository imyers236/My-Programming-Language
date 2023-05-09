//----------------------------------------------------------------------
// FILE: ast_parser.h
// DATE: CPSC 326, Spring 2023
// AUTH:
// DESC:
//----------------------------------------------------------------------

#ifndef AST_PARSER_H
#define AST_PARSER_H

#include "mypl_exception.h"
#include "lexer.h"
#include "ast.h"


class ASTParser
{
public:

  // crate a new recursive descent parer
  ASTParser(const Lexer& lexer);

  // run the parser
  Program parse();
  
private:
  
  Lexer lexer;
  Token curr_token;
  
  // helper functions
  void advance();
  void eat(TokenType t, const std::string& msg);
  bool match(TokenType t);
  bool match(std::initializer_list<TokenType> types);
  void error(const std::string& msg);
  bool bin_op();

  // recursive descent functions
  void struct_def(Program& p);
  void fun_def(Program& s);
  void fields(StructDef& s);
  void data_type(DataType& f);
  void base_type();
  void stmt(std::vector<std::shared_ptr<Stmt>>& s);
  void vdecl_stmt(VarDeclStmt& v);
  void assign_stmt(AssignStmt& a);
  void delete_struct_stmt(DeleteStructStmt& d);
  void delete_array_stmt(DeleteArrayStmt& d);
  void lvalue(std::vector<VarRef>& p);
  void if_stmt(IfStmt& i);
  void if_stmt_tail(IfStmt& i);
  void while_stmt(WhileStmt& w);
  void for_stmt(ForStmt& o);
  void call_expr(CallExpr& c);
  void ret_stmt(ReturnStmt& r);
  void expr(Expr& e);
  void rvalue(std::shared_ptr<RValue>& r);
  void new_rvalue(NewRValue& n);
  void base_rvalue(SimpleRValue& r);
  void var_rvalue(std::vector<VarRef>& p);
  void params(FunDef& f);

};


#endif
