#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdint>

#include "pin.H"

using std::map;
using std::ofstream;
using std::string;
using std::cout;
using std::cerr;
using std::hex;
using std::endl;


bool record = false;
ofstream outfile;
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "", "specify file name for MyPinTool output");

INT32 Usage() {
  cerr << "Usage of this tool : " 
    << "../../../pin -t obj-intel64/transtrace.so -- <binary to instrument>" 
    << endl;
  return -1;
}

VOID ReadAccess(ADDRINT addr) {
  if (record) 
    outfile << "r " << hex << addr << endl;
}

VOID WriteAccess(ADDRINT addr) {
  if (record)
    outfile << "w " << hex << addr << endl;
}

VOID Instruction(INS ins, void *v) {
  int cnt = INS_MemoryOperandCount(ins);

  for (int ii = 0; ii < cnt; ii++) {
    // Record read access
    if (INS_MemoryOperandIsRead(ins, ii)) {
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          (AFUNPTR) ReadAccess,
          IARG_MEMORYREAD_EA,
          IARG_END);
    }
    // Record write access
    else if (INS_MemoryOperandIsWrite(ins, ii)) {
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          (AFUNPTR) WriteAccess,
          IARG_MEMORYREAD_EA,
          IARG_END);
    }
  }
}

void RtnROIStart(int x) {
  record = true;
  cerr << "Pintools ROI instrumentation enabled" << endl;
}

void RtnROIEnd() {
  record = false;
  cerr << "Pintools ROI instrumentation disabled" << endl;
}

VOID Image(IMG img, VOID *v) {
/* outfile << "start image instrumentation" << endl; */

  RTN roistart = RTN_FindByName(img, "ROI_START");
  if (RTN_Valid(roistart)) {
    RTN_Open(roistart);
    RTN_InsertCall(roistart, 
                  IPOINT_AFTER, 
                  (AFUNPTR)RtnROIStart, 
                  IARG_FUNCRET_EXITPOINT_VALUE,
                  IARG_END);
    RTN_Close(roistart);
  }

  RTN roiend = RTN_FindByName(img, "ROI_END");
  if (RTN_Valid(roiend)) {
    RTN_Open(roiend);
    RTN_InsertCall(roiend, IPOINT_BEFORE, AFUNPTR(RtnROIEnd), IARG_END);
    RTN_Close(roiend);
  }
}

VOID Fini(int n, void *v) {
  cerr << "Finished instrumentation" << endl;
  outfile.close();
}

int main(int argc, char **argv) {

  // parse command line
  PIN_InitSymbols();
  if (PIN_Init(argc, argv)) 
  {
    return Usage();
  }

  outfile.open(KnobOutputFile.Value().c_str());

  // register image function
  IMG_AddInstrumentFunction(Image, 0);

  // register instrument function
  INS_AddInstrumentFunction(Instruction, 0);

  PIN_AddFiniFunction(Fini, 0);

  // start
  PIN_StartProgram();

  return 0;
}
