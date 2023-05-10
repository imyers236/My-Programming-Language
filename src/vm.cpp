//----------------------------------------------------------------------
// FILE: vm.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: 
// DESC: 
//----------------------------------------------------------------------

#include <iostream>
#include "vm.h"
#include "mypl_exception.h"


using namespace std;


void VM::error(string msg) const
{
  throw MyPLException::VMError(msg);
}


void VM::error(string msg, const VMFrame& frame) const
{
  int pc = frame.pc - 1;
  VMInstr instr = frame.info.instructions[pc];
  string name = frame.info.function_name;
  msg += " (in " + name + " at " + to_string(pc) + ": " +
    to_string(instr) + ")";
  throw MyPLException::VMError(msg);
}


string to_string(const VM& vm)
{
  string s = "";
  for (const auto& entry : vm.frame_info) {
    const string& name = entry.first;
    s += "\nFrame '" + name + "'\n";
    const VMFrameInfo& frame = entry.second;
    for (int i = 0; i < frame.instructions.size(); ++i) {
      VMInstr instr = frame.instructions[i];
      s += "  " + to_string(i) + ": " + to_string(instr) + "\n"; 
    }
  }
  return s;
}


void VM::add(const VMFrameInfo& frame)
{
  frame_info[frame.function_name] = frame;
}


void VM::run(bool DEBUG)
{
  // grab the "main" frame if it exists
  if (!frame_info.contains("main"))
    error("No 'main' function");
  shared_ptr<VMFrame> frame = make_shared<VMFrame>();
  frame->info = frame_info["main"];
  call_stack.push(frame);

  // run loop (keep going until we run out of instructions)
  while (!call_stack.empty() and frame->pc < frame->info.instructions.size()) {

    // get the next instruction
    VMInstr& instr = frame->info.instructions[frame->pc];

    // increment the program counter
    ++frame->pc;

    // for debugging
    if (DEBUG) {
      // TODO
      cerr << endl << endl;
      cerr << "\t FRAME.........: " << frame->info.function_name << endl;
      cerr << "\t PC............: " << (frame->pc - 1) << endl;
      cerr << "\t INSTR.........: " << to_string(instr) << endl;
      cerr << "\t NEXT OPERAND..: ";
      if (!frame->operand_stack.empty())
        cerr << to_string(frame->operand_stack.top()) << endl;
      else
        cerr << "empty" << endl;
      cerr << "\t NEXT FUNCTION.: ";
      if (!call_stack.empty())
        cerr << call_stack.top()->info.function_name << endl;
      else
        cerr << "empty" << endl;
    }

    //----------------------------------------------------------------------
    // Literals and Variables
    //----------------------------------------------------------------------

    if (instr.opcode() == OpCode::PUSH) {
      frame->operand_stack.push(instr.operand().value());
    }

    else if (instr.opcode() == OpCode::POP) {
      frame->operand_stack.pop();
    }

    
    else if(instr.opcode() == OpCode::STORE) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      int index = get<int>(instr.operand().value());
      if(index >= frame->variables.size())
      {
        frame->variables.push_back(x);
      }
      else
      {
        frame->variables.at(index) = x;
      }
      
    }
    
    else if(instr.opcode() == OpCode::LOAD) {
      VMValue x = frame->variables.at(get<int>(instr.operand().value()));
      frame->operand_stack.push(x);
    }
    
    // TODO: Finish LOAD and STORE

    
    //----------------------------------------------------------------------
    // Operations
    //----------------------------------------------------------------------

    else if (instr.opcode() == OpCode::ADD) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(add(y, x));
    }

    else if (instr.opcode() == OpCode::SUB) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(sub(y, x));
    }

    else if (instr.opcode() == OpCode::MUL) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(mul(y, x));
    }

    else if (instr.opcode() == OpCode::DIV) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(div(y, x));
    }

    else if (instr.opcode() == OpCode::AND) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(an(y, x));
    }

    else if (instr.opcode() == OpCode::OR) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(orr(y, x));
    }

    else if (instr.opcode() == OpCode::NOT) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      frame->operand_stack.push(nt(x));
    }

    else if (instr.opcode() == OpCode::CMPLT) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(lt(y, x));
    }

    else if (instr.opcode() == OpCode::CMPLE) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(le(y, x));
    }

    else if (instr.opcode() == OpCode::CMPGT) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(gt(y, x));
    }

    else if (instr.opcode() == OpCode::CMPGE) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(ge(y, x));
    }

    else if (instr.opcode() == OpCode::CMPEQ) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(eq(y, x));
    }
    
    else if (instr.opcode() == OpCode::CMPNE) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(neq(y, x));
    }
    
    //----------------------------------------------------------------------
    // Branching
    //----------------------------------------------------------------------

    else if (instr.opcode() == OpCode::JMP) {
      int index = get<int>(instr.operand().value());
      frame->pc = index;
    }

    else if (instr.opcode() == OpCode::JMPF) {
      VMValue checker = frame->operand_stack.top();
      frame->operand_stack.pop();
      int index = get<int>(instr.operand().value());
      if(holds_alternative<bool>(checker))
      {
        if(!get<bool>(checker))
        {
          frame->pc = index;
        }
      }
    }
    
    
    //----------------------------------------------------------------------
    // Functions
    //----------------------------------------------------------------------
    
    else if (instr.opcode() == OpCode::CALL)
    {
      string fun_name = get<string>(instr.operand().value());
      shared_ptr<VMFrame> new_frame = make_shared<VMFrame>();
      new_frame -> info = frame_info[fun_name];
      call_stack.push(new_frame);
      for(int i = 0; i < frame_info[fun_name].arg_count; i++)
      {
        VMValue x = frame -> operand_stack.top();
        new_frame -> operand_stack.push(x);
        frame -> operand_stack.pop();
      }
      frame = new_frame;
    }
    
    else if (instr.opcode() == OpCode::RET)
    {
      VMValue v = frame -> operand_stack.top();
      call_stack.pop();
      if(!call_stack.empty())
      {
        frame = call_stack.top();
        frame -> operand_stack.push(v);
      }
    }

    
    //----------------------------------------------------------------------
    // Built in functions
    //----------------------------------------------------------------------


    else if (instr.opcode() == OpCode::WRITE) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      cout << to_string(x);
    }

    else if (instr.opcode() == OpCode::READ) {
      string val = "";
      getline(cin, val);
      frame->operand_stack.push(val);
    }

    else if (instr.opcode() == OpCode::SLEN) {
      VMValue x1 = frame->operand_stack.top();
      ensure_not_null(*frame, x1);
      string x = get<string>(x1);
      frame->operand_stack.pop();
      int length = x.size();
      frame->operand_stack.push(length);
    }

    else if (instr.opcode() == OpCode::ALEN) {
      VMValue x1 = frame->operand_stack.top();
      ensure_not_null(*frame, x1);
      int x = get<int>(x1);
      frame->operand_stack.pop();
      vector<VMValue> val = array_heap[x];
      int length = val.size();
      frame->operand_stack.push(length);
    }

    else if (instr.opcode() == OpCode::TOINT) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      frame->operand_stack.push(to_int(x));
    }

    else if (instr.opcode() == OpCode::TODBL) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      frame->operand_stack.push(to_dbl(x));
    }

    else if (instr.opcode() == OpCode::TOSTR) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      frame->operand_stack.push(to_string(x));
    }

    else if (instr.opcode() == OpCode::CONCAT) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(con_cat(y,x));
    }

    else if (instr.opcode() == OpCode::GETC) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      string word = get<string>(x);
      frame->operand_stack.pop();
      x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      int index = get<int>(x);
      frame->operand_stack.pop();
      if(index >= word.size())
      {
        error("out-of-bounds string index (in main at 2: GETC())");
      }
      else if(index < 0)
      {
        error("out-of-bounds string index (in main at 2: GETC())");
      }
      else
      {
        string ch;
        ch.push_back(word[index]);
        frame->operand_stack.push(ch);
      }
    }
    
    
    //----------------------------------------------------------------------
    // heap
    //----------------------------------------------------------------------

    else if (instr.opcode() == OpCode::ALLOCS) {
      struct_heap[next_obj_id] = {};
      frame->operand_stack.push(next_obj_id);
      ++next_obj_id;
    }

    else if (instr.opcode() == OpCode::ADDF) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int oid = get<int>(x);
      struct_heap[oid][get<string>(instr.operand().value())];
    }

    else if (instr.opcode() == OpCode::SETF) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      int oid = get<int>(y);
      struct_heap[oid][get<string>(instr.operand().value())] = x;
    }

    else if (instr.opcode() == OpCode::GETF) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, x);
      int oid = get<int>(x);
      if(struct_heap[oid].empty())
      {
        error("struct does not exist (in main at 1: GETF(a))");
      }
      else
      {
        frame->operand_stack.push(struct_heap[oid][get<string>(instr.operand().value())]);
      }
    }

    else if (instr.opcode() == OpCode::ALLOCA) {
      VMValue val = frame->operand_stack.top();
      frame->operand_stack.pop();
      int size = get<int>(frame->operand_stack.top());
      frame->operand_stack.pop();
      array_heap[next_obj_id] = vector<VMValue>(size,val);
      frame->operand_stack.push(next_obj_id);
      ++next_obj_id;
    }

    else if (instr.opcode() == OpCode::SETI) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      VMValue z = frame->operand_stack.top();
      ensure_not_null(*frame, z);
      frame->operand_stack.pop();
      if(array_heap[get<int>(z)].empty())
      {
        error("array does not exist (in main at 5: SETI())");
      }
      else if(get<int>(y) < array_heap[get<int>(z)].size())
      {
        if(get<int>(y) < 0)
        {
          error("out-of-bounds array index (in main at 5: SETI())");
        }
        array_heap[get<int>(z)][get<int>(y)] = x;
      }
      else
      {
        error("out-of-bounds array index (in main at 5: SETI())");
      }
    }

    else if (instr.opcode() == OpCode::GETI) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      if(array_heap[get<int>(y)].empty())
      {
        error("array does not exist (in main at 4: GETI())");
      }
      else if(get<int>(x) < array_heap[get<int>(y)].size())
      {
        if(get<int>(y) < 0)
        {
          error("out-of-bounds array index (in main at 4: GETI())");
        }
        frame->operand_stack.push(array_heap[get<int>(y)][get<int>(x)]);
      }
      else
      {
        error("out-of-bounds array index (in main at 4: GETI())");
      }
    }
    
    else if (instr.opcode() == OpCode::DELAR) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      array_heap.erase (get<int>(x));
    }

    else if (instr.opcode() == OpCode::DELS) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      struct_heap.erase(get<int>(x));
      /*
      std::unordered_map<int, std::unordered_map<std::string, VMValue>> temp;
      for(int i = 2023; i < next_obj_id; i++)
      {
        if(!struct_heap[i].empty())
        {
          temp[i] = struct_heap[i];
        }
      }
      struct_heap = temp;
      */
      //int y = struct_heap.size();
      //auto itr = struct_heap[get<int>(x)];
      //itr.clear();
      //struct_heap.erase(get<int>(x));
      //int z = struct_heap.size();
      //struct_heap.rehash(z);
      //error("size before: " + to_string(y) + " size after: " + to_string(z));
      //struct_heap.free(get<int>(x));
    }
    //----------------------------------------------------------------------
    // special
    //----------------------------------------------------------------------

    
    else if (instr.opcode() == OpCode::DUP) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(x);
      frame->operand_stack.push(x);      
    }

    else if (instr.opcode() == OpCode::NOP) {
      // do nothing
    }
    
    else {
      error("unsupported operation " + to_string(instr));
    }
  }
}


void VM::ensure_not_null(const VMFrame& f, const VMValue& x) const
{
  if (holds_alternative<nullptr_t>(x))
    error("null reference", f);
}


VMValue VM::add(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) + get<int>(y);
  else
    return get<double>(x) + get<double>(y);
}

VMValue VM::sub(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) - get<int>(y);
  else
    return get<double>(x) - get<double>(y);
}

VMValue VM::mul(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) * get<int>(y);
  else
    return get<double>(x) * get<double>(y);
}

VMValue VM::div(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) / get<int>(y);
  else
    return get<double>(x) / get<double>(y);
}

VMValue VM::an(const VMValue& x, const VMValue& y) const
{ 
    return get<bool>(x) && get<bool>(y);
}

VMValue VM::orr(const VMValue& x, const VMValue& y) const
{ 
    return get<bool>(x) || get<bool>(y);
}

VMValue VM::nt(const VMValue& x) const
{ 
    return !get<bool>(x);
}

VMValue VM::eq(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) == get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) == get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) == get<string>(y);
  else
    return get<bool>(x) == get<bool>(y);
}

VMValue VM::neq(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return true;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) != get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) != get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) != get<string>(y);
  else
    return get<bool>(x) != get<bool>(y);
}

// TODO: Finish the rest of the comparison operators

VMValue VM::lt(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) < get<int>(y);
  else if(holds_alternative<double>(x))
    return get<double>(x) < get<double>(y);
  else
    return get<string>(x) < get<string>(y);
}

VMValue VM::le(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) <= get<int>(y);
  else if(holds_alternative<double>(x))
    return get<double>(x) <= get<double>(y);
  else
    return get<string>(x) <= get<string>(y);
}

VMValue VM::gt(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) > get<int>(y);
  else if(holds_alternative<double>(x))
    return get<double>(x) > get<double>(y);
  else
    return get<string>(x) > get<string>(y);
}

VMValue VM::ge(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) >= get<int>(y);
  else if(holds_alternative<double>(x))
    return get<double>(x) >= get<double>(y);
  else
    return get<string>(x) >= get<string>(y);
}

VMValue VM::to_int(const VMValue& x) const
{
  if (holds_alternative<double>(x)) 
  {
    int y = (int) get<double>(x);
    return y;
  }
  else if(holds_alternative<string>(x))
  {
    try{
      int y = stoi(get<string>(x));
      return y;
    }
    catch(exception &err){
      error("cannot convert string to int (in main at 1: TOINT())");
    }
  }
}

VMValue VM::to_dbl(const VMValue& x) const
{
  if (holds_alternative<int>(x)) 
  {
    double y = (double) get<int>(x);
    return y;
  }
  else if(holds_alternative<string>(x))
  {
    try{
      double y = stod(get<string>(x));
      return y;
    }
    catch(exception &err){
      error("cannot convert string to double (in main at 1: TODBL())");
    }
  }
}

VMValue VM::to_str(const VMValue& x) const
{
  if (holds_alternative<int>(x)) 
  {
    return get<string>(x);
  }
  else if(holds_alternative<double>(x))
  {
    return get<string>(x);
  }
}

VMValue VM::con_cat(const VMValue& x, const VMValue& y) const
{
  return get<string>(x) + get<string>(y);
}

