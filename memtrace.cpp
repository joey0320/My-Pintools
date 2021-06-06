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
string outfilename = "memtrace.out";

INT32 Usage() {
  cerr << "this tool traces the memory access of your binary" << endl;
  cerr << "../../../pin -t obj-intel64/memtrace.so -- <your bin>" << endl;
  return -1;
}

VOID RecordReadAccess(ADDRINT addr) {
  outfile << 0 << " " << hex << addr << endl;
}

VOID RecordInstAccess(ADDRINT addr) {
  outfile << 2 << " " << hex << addr << endl;
}

VOID RecordWriteAccess(ADDRINT addr) {
  outfile << 1 << " " << hex << addr << endl;
}

VOID Instruction(INS ins, void *v) {
  // memory operations in instruction
  int mem_cnt = INS_MemoryOperandCount(ins);

  for (int ii = 0; ii < mem_cnt; ii++) {
    // if the memory op is read
    if (INS_MemoryOperandIsRead(ins, ii)) {
      INS_InsertCall(ins, 
          /* insert code before inst */ IPOINT_BEFORE, 
          /* code to insert */ (AFUNPTR) RecordReadAccess,
          /* args : look at pintools API */ IARG_MEMORYOP_EA, ii, 
          /* end of args */ IARG_END);
    }
    // if the memory op is write
    else if (INS_MemoryOperandIsWritten(ins, ii)) {
      INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) RecordWriteAccess,
          IARG_MEMORYOP_EA, ii, IARG_END);
    }
  }

  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) RecordInstAccess,
      IARG_INST_PTR, IARG_END);
}

VOID Fini(int n, void *v) {
  cerr << "finished  ! " << endl;
  outfile.close();
}

int main (int argc, char *argv[]) {

  // parse command line
  if (PIN_Init(argc, argv)) 
  {
    return Usage();
  }

  outfile.open(outfilename.c_str());

  // register instrument function
  INS_AddInstrumentFunction(Instruction, 0);
  PIN_AddFiniFunction(Fini, 0);

  // start
  PIN_StartProgram();

  return 0;
}

