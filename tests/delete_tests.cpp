//----------------------------------------------------------------------
// FILE: delete_tests.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: S. Bowers
// DESC: Basic code generator tests
//----------------------------------------------------------------------



#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "mypl_exception.h"
#include "lexer.h"
#include "ast_parser.h"
#include "semantic_checker.h"
#include "vm.h"
#include "code_generator.h"

using namespace std;


streambuf* stream_buffer;


void change_cout(stringstream& out)
{
  stream_buffer = cout.rdbuf();
  cout.rdbuf(out.rdbuf());
}

void restore_cout()
{
  cout.rdbuf(stream_buffer);
}

string build_string(initializer_list<string> strs)
{
  string result = "";
  for (string s : strs)
    result += s + "\n";
  return result;
}

//----------------------------------------------------------------------
// token tests
//----------------------------------------------------------------------

TEST(DeleteTests, TokenCreation) {
  Token token(TokenType::DELETE, "delete", 2, 2);
}

TEST(DeleteTests, TypeGivenIsReturned) {
  Token token(TokenType::DELETE, "delete", 0, 0);
  ASSERT_EQ(TokenType::DELETE, token.type());
}

TEST(DeleteTests, LexemeGivenIsReturned) {
  Token token(TokenType::DELETE, "delete", 0, 0);
  ASSERT_EQ("delete", token.lexeme());
}

TEST(DeleteTests, CorrectlyPrints) {
  Token token(TokenType::DELETE, "delete", 4, 2);
  ASSERT_EQ("4, 2: DELETE 'delete'", to_string(token));
}

//----------------------------------------------------------------------
// lexer tests
//----------------------------------------------------------------------

TEST(DeleteTests, ComplexTypeReservedWords) {
  stringstream in("struct array delete");
  Lexer lexer(in);
  Token t = lexer.next_token();
  ASSERT_EQ(TokenType::STRUCT, t.type());
  ASSERT_EQ("struct", t.lexeme());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(1, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::ARRAY, t.type());
  ASSERT_EQ("array", t.lexeme());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(8, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::DELETE, t.type());
  ASSERT_EQ("delete", t.lexeme());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(14, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::EOS, t.type());
}

//----------------------------------------------------------------------
// ASTParser tests
//----------------------------------------------------------------------

TEST(DeleteTests, DeleteStructInAST) {
  stringstream in(build_string({
        "struct S {int x, bool y, array string z}",
        "void main() {",
        "  S s = null" 
        "  delete s",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  ASSERT_EQ(3, p.struct_defs[0].fields.size());
  ASSERT_EQ("int", p.struct_defs[0].fields[0].data_type.type_name);
  ASSERT_FALSE(p.struct_defs[0].fields[0].data_type.is_array);
  ASSERT_EQ("x", p.struct_defs[0].fields[0].var_name.lexeme());
  ASSERT_EQ("bool", p.struct_defs[0].fields[1].data_type.type_name);
  ASSERT_FALSE(p.struct_defs[0].fields[1].data_type.is_array);
  ASSERT_EQ("y", p.struct_defs[0].fields[1].var_name.lexeme());
  ASSERT_EQ("string", p.struct_defs[0].fields[2].data_type.type_name);
  ASSERT_TRUE(p.struct_defs[0].fields[2].data_type.is_array);
  ASSERT_EQ("z", p.struct_defs[0].fields[2].var_name.lexeme());
  ASSERT_EQ(2, p.fun_defs[0].stmts.size());
  VarDeclStmt& s = (VarDeclStmt&)*p.fun_defs[0].stmts[0];
  ASSERT_FALSE(s.var_def.data_type.is_array);
  ASSERT_EQ("S", s.var_def.data_type.type_name);
  ASSERT_EQ("s", s.var_def.var_name.lexeme());
  Expr& e = ((DeleteStmt&)*p.fun_defs[0].stmts[1]).expr;
  VarRValue& v = (VarRValue&)*((SimpleTerm&)*e.first).rvalue;
  ASSERT_EQ("s",v.path[0].var_name.lexeme());
}

TEST(DeleteTests, DeleteArrayInAST) {
  stringstream in(build_string({
        "void main() {",
        "  array int x = new int[4]" 
        "  delete x",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  ASSERT_EQ(2, p.fun_defs[0].stmts.size());
  VarDeclStmt& s = (VarDeclStmt&)*p.fun_defs[0].stmts[0];
  ASSERT_TRUE(s.var_def.data_type.is_array);
  ASSERT_EQ("int", s.var_def.data_type.type_name);
  ASSERT_EQ("x", s.var_def.var_name.lexeme());
  Expr& e = ((DeleteStmt&)*p.fun_defs[0].stmts[1]).expr;
  VarRValue& v = (VarRValue&)*((SimpleTerm&)*e.first).rvalue;
  ASSERT_EQ("x",v.path[0].var_name.lexeme());
}

//----------------------------------------------------------------------
// Semantic Checker tests
//----------------------------------------------------------------------

TEST(DeleteTests, DeleteStructPositiveSemantic) {
  stringstream in(build_string({
        "struct S1 {int x, bool y}",
        "void main() {",
        "S1 s = new S1",
        "delete s"
        "}",
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(DeleteTests, DeleteArrayPositiveSemantic) {
  stringstream in(build_string({
        "void main() {",
        "array int x = new int[4]",
        "delete x"
        "}",
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(DeleteTests, DeleteStructNegativeSemantic) {
  stringstream in(build_string({
        "void main() {delete s}",
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch (MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}

TEST(DeleteTests, DeleteArrayNegativeSemantic) {
  stringstream in(build_string({
        "void main() {",
        "int x = 5",
        "delete x"
        "}",
      }));
  SemanticChecker checker;
  try {
    ASTParser(Lexer(in)).parse().accept(checker);
    FAIL();
  } catch (MyPLException& ex) {
    string msg = ex.what();
    ASSERT_TRUE(msg.starts_with("Static Error:"));
  }
}
//----------------------------------------------------------------------
// Simple getting started tests
//----------------------------------------------------------------------

TEST(DeleteTests, EmptyProram) {
  stringstream in("void main() {}");
  VM vm;
  CodeGenerator generator(vm);
  ASTParser(Lexer(in)).parse().accept(generator);
  vm.run();
}




//----------------------------------------------------------------------
// main
//----------------------------------------------------------------------

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}