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

TEST(DeleteTests, TokenCreationStruct) {
  Token token(TokenType::DELETESTRUCT, "delete_struct", 2, 2);
}

TEST(DeleteTests, TokenCreationArray) {
  Token token(TokenType::DELETEARRAY, "delete_array", 2, 2);
}

TEST(DeleteTests, TypeGivenIsReturnedStruct) {
  Token token(TokenType::DELETESTRUCT, "delete_struct", 0, 0);
  ASSERT_EQ(TokenType::DELETESTRUCT, token.type());
}

TEST(DeleteTests, TypeGivenIsReturnedArray) {
  Token token(TokenType::DELETEARRAY, "delete_array", 0, 0);
  ASSERT_EQ(TokenType::DELETEARRAY, token.type());
}

TEST(DeleteTests, LexemeGivenIsReturnedStruct) {
  Token token(TokenType::DELETESTRUCT, "delete_struct", 0, 0);
  ASSERT_EQ("delete_struct", token.lexeme());
}

TEST(DeleteTests, LexemeGivenIsReturnedArray) {
  Token token(TokenType::DELETEARRAY, "delete_array", 0, 0);
  ASSERT_EQ("delete_array", token.lexeme());
}

TEST(DeleteTests, CorrectlyPrintsStruct) {
  Token token(TokenType::DELETESTRUCT, "delete_struct", 4, 2);
  ASSERT_EQ("4, 2: DELETESTRUCT 'delete_struct'", to_string(token));
}

TEST(DeleteTests, CorrectlyPrintsArray) {
  Token token(TokenType::DELETEARRAY, "delete_array", 4, 2);
  ASSERT_EQ("4, 2: DELETEARRAY 'delete_array'", to_string(token));
}

//----------------------------------------------------------------------
// lexer tests
//----------------------------------------------------------------------

TEST(DeleteTests, ComplexTypeReservedWords) {
  stringstream in("struct array delete_struct delete_array");
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
  ASSERT_EQ(TokenType::DELETESTRUCT, t.type());
  ASSERT_EQ("delete_struct", t.lexeme());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(14, t.column());
  t = lexer.next_token();
  ASSERT_EQ(TokenType::DELETEARRAY, t.type());
  ASSERT_EQ("delete_array", t.lexeme());
  ASSERT_EQ(1, t.line());
  ASSERT_EQ(28, t.column());
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
        "  delete_struct s",
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
  Expr& e = ((DeleteStructStmt&)*p.fun_defs[0].stmts[1]).expr;
  VarRValue& v = (VarRValue&)*((SimpleTerm&)*e.first).rvalue;
  ASSERT_EQ("s",v.path[0].var_name.lexeme());
}

TEST(DeleteTests, DeleteArrayInAST) {
  stringstream in(build_string({
        "void main() {",
        "  array int x = new int[4]" 
        "  delete_array x",
        "}"
      }));
  Program p = ASTParser(Lexer(in)).parse();
  ASSERT_EQ(2, p.fun_defs[0].stmts.size());
  VarDeclStmt& s = (VarDeclStmt&)*p.fun_defs[0].stmts[0];
  ASSERT_TRUE(s.var_def.data_type.is_array);
  ASSERT_EQ("int", s.var_def.data_type.type_name);
  ASSERT_EQ("x", s.var_def.var_name.lexeme());
  Expr& e = ((DeleteArrayStmt&)*p.fun_defs[0].stmts[1]).expr;
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
        "delete_struct s"
        "}",
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(DeleteTests, DeleteArrayPositiveSemantic) {
  stringstream in(build_string({
        "void main() {",
        "array int x = new int[4]",
        "delete_array x"
        "}",
      }));
  SemanticChecker checker;
  ASTParser(Lexer(in)).parse().accept(checker);
}

TEST(DeleteTests, DeleteStructNegativeSemantic) {
  stringstream in(build_string({
        "void main() {delete_struct s}",
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
        "delete_array x"
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
// VM tests
//----------------------------------------------------------------------

TEST(DeleteTests, DeleteOneFieldStruct) {
  VMFrameInfo main {"main", 0};                      
  main.instructions.push_back(VMInstr::ALLOCS());
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::ADDF("field_1"));
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::PUSH("blue"));
  main.instructions.push_back(VMInstr::SETF("field_1"));
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::DELS());
  main.instructions.push_back(VMInstr::GETF("field_1"));
  VM vm;
  vm.add(main);
  stringstream out;
  change_cout(out);
  try {
    vm.run();
    FAIL();
  } catch(MyPLException& ex) {
    string err = ex.what();
    string msg = "VM Error: struct does not exist ";
    msg += "(in main at 1: GETF(a))";
    EXPECT_EQ(msg, err);
  }
  restore_cout();
}

TEST(DeleteTests, DeleteTwoFieldStruct) {
  VMFrameInfo main {"main", 0};                      
  main.instructions.push_back(VMInstr::ALLOCS());
  main.instructions.push_back(VMInstr::STORE(0));     // x = oid_1
  main.instructions.push_back(VMInstr::LOAD(0));
  main.instructions.push_back(VMInstr::ADDF("field_1"));
  main.instructions.push_back(VMInstr::LOAD(0));
  main.instructions.push_back(VMInstr::ADDF("field_2"));
  main.instructions.push_back(VMInstr::LOAD(0));
  main.instructions.push_back(VMInstr::PUSH("blue"));
  main.instructions.push_back(VMInstr::SETF("field_1"));
  main.instructions.push_back(VMInstr::LOAD(0));
  main.instructions.push_back(VMInstr::PUSH("green"));
  main.instructions.push_back(VMInstr::SETF("field_2"));
  main.instructions.push_back(VMInstr::LOAD(0));  
  main.instructions.push_back(VMInstr::GETF("field_1"));
  main.instructions.push_back(VMInstr::WRITE());
  main.instructions.push_back(VMInstr::LOAD(0)); 
  main.instructions.push_back(VMInstr::DELS()); 
  main.instructions.push_back(VMInstr::LOAD(0));  
  main.instructions.push_back(VMInstr::GETF("field_2"));
  VM vm;
  vm.add(main);
  stringstream out;
  change_cout(out);
  try {
    vm.run();
    FAIL();
  } catch(MyPLException& ex) {
    string err = ex.what();
    string msg = "VM Error: struct does not exist ";
    msg += "(in main at 1: GETF(a))";
    EXPECT_EQ(msg, err);
  }
  restore_cout();
}

TEST(DeleteTests, BasicArrayDeleteGet) {
  VMFrameInfo main {"main", 0};                      
  main.instructions.push_back(VMInstr::PUSH(5));     // length
  main.instructions.push_back(VMInstr::PUSH(0));     // fill with 5 0's
  main.instructions.push_back(VMInstr::ALLOCA());     // index
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::DELAR());     
  main.instructions.push_back(VMInstr::PUSH(0));     // index
  main.instructions.push_back(VMInstr::GETI());  
  VM vm;
  vm.add(main);
  stringstream out;
  change_cout(out);
  try {
    vm.run();
    FAIL();
  } catch(MyPLException& ex) {
    string err = ex.what();
    string msg = "VM Error: array does not exist ";
    msg += "(in main at 4: GETI())";
    EXPECT_EQ(msg, err);
  }
  restore_cout();
}

TEST(DeleteTests, BasicArrayDeleteSet) {
  VMFrameInfo main {"main", 0};                      
  main.instructions.push_back(VMInstr::PUSH(5));     // length
  main.instructions.push_back(VMInstr::PUSH(0));     // fill with 5 0's
  main.instructions.push_back(VMInstr::ALLOCA());     // index
  main.instructions.push_back(VMInstr::DUP());
  main.instructions.push_back(VMInstr::DELAR());     
  main.instructions.push_back(VMInstr::PUSH(0));     
  main.instructions.push_back(VMInstr::PUSH(6));     
  main.instructions.push_back(VMInstr::SETI());  
  VM vm;
  vm.add(main);
  stringstream out;
  change_cout(out);
  try {
    vm.run();
    FAIL();
  } catch(MyPLException& ex) {
    string err = ex.what();
    string msg = "VM Error: array does not exist ";
    msg += "(in main at 5: SETI())";
    EXPECT_EQ(msg, err);
  }
  restore_cout();
}
//----------------------------------------------------------------------
// Code Generator tests
//----------------------------------------------------------------------

TEST(DeleteTests, EmptyProram) {
  stringstream in("void main() {}");
  VM vm;
  CodeGenerator generator(vm);
  ASTParser(Lexer(in)).parse().accept(generator);
  vm.run();
}

TEST(DeleteTests, DeleteStruct) {
  stringstream in(build_string({
        "struct T {int x, bool y}",
        "void main() {",
        "  T t = new T",
        "  t.x = 42",
        "  t.y = true",
        "  print(t.x)",
        "  print(t.y)",
        "  delete_struct t",
        "}"
      }));
  VM vm;
  CodeGenerator generator(vm);
  ASTParser(Lexer(in)).parse().accept(generator);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("42true", out.str());
  restore_cout();
}

TEST(DeleteTests, DeleteArray) {
  stringstream in(build_string({
        "void main() {",
        "  array bool xs = new bool[3]", 
        "  xs[0] = false",
        "  xs[1] = true",
        "  print(xs[0])",
        "  print(xs[1])",
        "  print(xs[2])",
        "  delete_array xs",
        "}"
      }));
  VM vm;
  CodeGenerator generator(vm);
  ASTParser(Lexer(in)).parse().accept(generator);
  stringstream out;
  change_cout(out);
  vm.run();
  EXPECT_EQ("falsetruenull", out.str());
  restore_cout();
}




//----------------------------------------------------------------------
// main
//----------------------------------------------------------------------

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}