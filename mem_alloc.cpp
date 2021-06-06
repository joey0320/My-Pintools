#include "pin.H"
#include <iostream>
#include <fstream>
#include <map>
#include <string>


#if defined(TARGET_MAC)
#define MALLOC "_malloc"
#define FREE "_free"
#else
#define MALLOC "malloc"
#define FREE "free"
#endif
using std::string;

std::map<ADDRINT, bool> mallocmap;
std::ofstream outfile;
std::string programimage;
/* std::string outfilename = "memtrace_out.txt"; */
KNOB<string> outfilename(KNOB_MODE_WRITEONCE,
"pintool", "o",
"memtrace_out.txt",
"Memory trace file name");

INT32 Usage()
{
    std::cerr <<
        "This tool produces a trace of calls to malloc.\n"
        "\n";

    std::cerr << KNOB_BASE::StringKnobSummary();

    std::cerr << std::endl;

    return -1;
}

VOID RecordMalloc(ADDRINT addr) {
  if (addr == 0x0) {
    std::cerr << "Heap full";
    return;
  }

  std::map<ADDRINT, bool>::iterator it = mallocmap.find(addr);

  // found addr that was allocated
  if (it != mallocmap.end()) {
    // address was allocated then freed
    if (it->second) {
      it->second = false;
    }
    // address is allocated
    else {
      outfile << "impossible : \
        trying to allocate where already allocated" << std::endl;
    }
  }
  // first time allocating to this address
  else {
    mallocmap.insert(std::pair<ADDRINT, bool>(addr, false));
  }
}

VOID RecordFree(ADDRINT addr) {

  std::map<ADDRINT, bool>::iterator it = mallocmap.find(addr);

  // memory is allocated
  if (it != mallocmap.end()) {
    // memory was previously freed
    if (it->second) {
      outfile << "cannot free already freed at addr : " <<
        std::hex << addr << std::endl;
      return;
    }
    // memory is not freed : free it
    else {
      it->second = true;
    }
  }
  // memory is not allocated
  else {
    outfile << "impossible : \
      cannot free memory that is not allocated at addr : " <<
     std::hex << addr << std::endl;
  }
}


VOID Image(IMG img, VOID *v) {

  // find rtn by name
  RTN mallocRtn = RTN_FindByName(img, MALLOC);

  // if the rtn is valid
  if (RTN_Valid(mallocRtn)) {
    // open the rtn
    RTN_Open(mallocRtn);

    // inject analysis code
    RTN_InsertCall(
        /* rtn to insert code */ mallocRtn, 
        /* where to insert? */ IPOINT_AFTER, 
        /* what to insert? */ (AFUNPTR)RecordMalloc, 
        /* analysis code args */ IARG_FUNCRET_EXITPOINT_VALUE,
        /* notify end of args */ IARG_END);

    // close the rtn
    RTN_Close(mallocRtn);
  }

  // find rtn by name
  RTN freeRtn = RTN_FindByName(img, FREE);

  // if the rtn is valid
  if (RTN_Valid(freeRtn)) {
    // open the rtn
    RTN_Open(freeRtn);

    // inject analysis code
    RTN_InsertCall(
        /* rtn to insert code */ freeRtn, 
        /* where to insert? */ IPOINT_BEFORE, 
        /* what to insert? */ (AFUNPTR)RecordFree, 
        /* analysis code args */ IARG_FUNCARG_ENTRYPOINT_VALUE,
        /* index of arguement to FREE */ 0,
        /* notify end of args */ IARG_END);

    // close the rtn
    RTN_Close(freeRtn);
  }
}

VOID Fini(INT32 code, VOID *v) {
  for (std::pair<ADDRINT, bool> p : mallocmap) {
    if (!p.second) {
      outfile << "memory at " << std::hex << p.first <<
        "allocated but not freed" << std::endl;
    }
  }
  outfile.close();
}

int main (int argc, char **argv) {
  // since we are searching for function names
  // we need to load the program symbol table
  PIN_InitSymbols();

  // parse the command line to obtain KNOBs
  if (PIN_Init(argc, argv) )
    return Usage();

  // assume that the image name is always at index 6
  programimage = argv[6];

  std::cout << programimage << std::endl;
  

/* outfile.open(outfilename.c_str()); */
outfile.open(outfilename.Value().c_str());
/* outfile << std::hex; */
/* outfile.setf(std::ios::showbase); */

/* std::cout << outfilename << std::endl; */

  // registers the instrument function Image
  // to be called every time a image is loaded
  IMG_AddInstrumentFunction(Image, 0);

  // the second argument can be used to pass any additional 
  // information to instrumentation functions
  PIN_AddFiniFunction(Fini, 0);

  // start the program to perform analysis
  PIN_StartProgram();

  return 0;
}
