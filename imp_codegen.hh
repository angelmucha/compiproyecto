#ifndef IMP_CODEGEN
#define IMP_CODEGEN

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <fstream>

#include "imp.hh"
#include "imp_visitor.hh"
#include "environment.hh"
#include <stack>

class ImpCodeGen : public ImpVisitor {
public:
  void codegen(Program*, string outfname);
  int visit(Program*);
  int visit(Body*);
  int visit(VarDecList*);
  int visit(VarDec*);
  int visit(StatementList*);
  int visit(AssignStatement*);
  int visit(PrintStatement*);
  int visit(IfStatement*);
  int visit(WhileStatement*);
  int visit(ForStatement*);
  int visit(DoWhileStatement*);
  int visit(BreakStatement*);
  int visit(ContinueStatement*);
  
  int visit(BinaryExp* e);
  int visit(UnaryExp* e);
  int visit(NumberExp* e);
  int visit(BoolConstExp* e);
  int visit(IdExp* e);
  int visit(ParenthExp* e);
  int visit(CondExp* e);
  void pushWhileExitLabel(const string& label) { whileExitLabels.push(label); }
  void popWhileExitLabel() { whileExitLabels.pop(); }
  string topWhileExitLabel() const { return whileExitLabels.top(); }

  void pushDoWhileExitLabel(const string& label) { doWhileExitLabels.push(label); }
  void popDoWhileExitLabel() { doWhileExitLabels.pop(); }
  string topDoWhileExitLabel() const { return doWhileExitLabels.top(); }

private:
  stack<string> whileExitLabels;
  stack<string> doWhileExitLabels;
  std::ostringstream code;
  string nolabel;
  int current_label;
  Environment<int> direcciones;
  int siguiente_direccion, mem_locals;
  void codegen(string label, string instr);
  void codegen(string label, string instr, int arg);
  void codegen(string label, string instr, string jmplabel);
  string next_label();
};


#endif

