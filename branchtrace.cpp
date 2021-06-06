/* branchtrace.cpp */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <string>

using std::cout;
using std::cerr;
using std::string;
using std::hex;
using std::endl;
using std::ofstream;

ofstream outfile;
string outfilename = "branchtrace.out";

INT32 Usage() {
  cerr << "this tool traces the branch instructions of your binary" << endl;
  cerr << "../../../pin -t obj-intel64/branchtrace.so -- <your bin>" << endl;
  return -1;
}

VOID RecordBranch(ADDRINT PC, ADDRINT NEXT_PC, bool taken) {
  outfile << hex << PC << " " 
          << hex <<  NEXT_PC << " "
          << taken << endl;
}

// IARG_INST_PTR
VOID Instruction(INS ins, void *v) {
  // not branch
  if (!INS_IsBranch(ins)) 
    return;

  // branch inst
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) RecordBranch,
      IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, 
      IARG_END);
}

VOID Fini(int n, void *v) {
  cerr << "finished !" << endl;
  outfile.close();
}

int main (int argc, char *argv[]) {

  if (PIN_Init(argc, argv))
  {
    return Usage();
  }

  outfile.open(outfilename.c_str());

  INS_AddInstrumentFunction(Instruction, 0);
  PIN_AddFiniFunction(Fini, 0);

  PIN_StartProgram();

  return 0;
} 
