#include <verilated.h>             // Defines common routines
#include "Vassign2.h"              // From Verilating "assign2.1.v"
#ifdef COVERED_INLINED
#include "Vasignt2_main.h"
#endif
#include <SpTraceVcdC.h>           // Trace file format header (from SystemPerl)

Vassign2 *top;                        // Instantiation of module

unsigned int main_time = 0;        // Current simulation time

double sc_time_stamp () {          // Called by $time in Verilog
  return main_time;
}

int main() {

  top = new Vassign2;                 // Create instance

  Verilated::traceEverOn( true );  // Verilator must compute traced signals
  SpTraceVcdCFile* tfp = new SpTraceVcdCFile;
  top->trace( tfp, 99 );           // Trace 99 levels of hierarchy
  tfp->open( "assign2.1.vcd" );         // Open the dump file

#ifdef COVERED_INLINED
  covered_initialize( top, "../assign2.1.cdd" );
#endif

  top->verilatorclock = 0;

  while( !Verilated::gotFinish() ) {
    top->verilatorclock = (main_time % 2);   // Toggle clock
    top->eval();                   // Evaluate model
    tfp->dump( main_time );        // Create waveform trace for this timestamp
    main_time++;                   // Time passes...
  }

  top->final();                    // Done simulating

#ifdef COVERED_INLINED
  covered_close( "../assign2.1.cdd" );
#endif

  tfp->close();

}

