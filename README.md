
# VIP_Static_Timing_analyzer
VIP ST analyzer is a Static Timing Analysis (STA) tool that calculates and displays timing information for the analyzed Verilog IP.
This first version of the tool reports circuit delay, slack values, and identifies the critical path.
The following modules are included, some of which are incomplete and their commands are not fully supported and used:
Input Verilog/SystemVerilog file that describes the gate-level netlist of the IP. The IP must be elaborated with a flat hierarchy. All instances are of cells from the standard library.
The Verifik parser can produce such an elaborate gate-level netlist from cells from a full hierarchical Verilog design. This step is missing in this tool due to a limitation in the parser code.
Parser of a standard cell library in Liberty format.
Have included two such parsers - one which is very simplified with only a few options and the second in the ./LIB_parser folder which supports almost all of the Liberty syntax for describing standard cells.       
SDC parser - reads a Synopsys Design Constraints(https://www.synopsys.com/content/dam/synopsys/verification/white-papers/tcm-wp.pdf) file and currently only a few timing commands are available:
```javascript
    -create_clock -name -period [get_ports ]
    -set_input_delay value -max/min -fall/rise [get_ports ] -clock
    -set_input_transition value -max/min -fall/rise [get_ports] -clock
    -set_load -pin_load value [get_ports ]
    -set_output_delay value -max/min -fall/rise [get_ports ] -clock

```
Outputs are printed to the console and include results for the following timings:
```javascript
    Circuit Delay: Total circuit delay in picoseconds.
    Gate Slacks: Slack values ​​for each gate.
    Critical Path: Sequence of gates in the critical path.
```
The analysis of the IP design graph is also simplified because the elaboration step producing the full design gate-level netlist that is performed by the Verify parser is missing.
Since the Verific library is a commercial source that is paid for, it is not included here.
For information about the Verific Verilog parser and library, see www.verific.com.


## Build
Open the provided .pro file in QTCreator and run the build.
Tested and built in Windows 10, it should open and convert without problems in Linux(Ubuntu,Fedora etc.).
For a successful build, you need the full source code of Verific library.
Under Linux, after opening the .pro file and run qmake, a full Makefile is written, which can also be built with the make command.
## Usage

```javascript
Usage: VIP Static timing analyzer -v <input Verilog file> -lib <standard cells file> -sdc <SDC file> 
        -v file     \t...\t verilog file\n
        -lib file   \t...\t library file\n
        -sdc file   \t...\t SDC file

```


## Test
Located in the ./test_designs folder is a simple Verilog IP used as a standard benchmark for STA analysis named c17.v
There is also an example SDC file simple.sdc and two files with different descriptions of standard cells - one with a full Liberty description named std_cell_simplified.lib
and the other is the one used for the tests and is named NLDM_max2Inp.lib. It has a simplified description of cells in Liberty syntax.
## Run test

```javascript
        VIP_ST_analyzer  -v ./test_designs/c17.v  -sdc ./test_designs/simple.sdc -lib ./test_designs/NLDM_max2Inp.lib
```

## Example log 

```javascript
-- Reading structural Verilog file 'XXXXXXXXXXXXXXX\veri_test\c17.v' (VNLR-1084)
F:\XXXXXXXXXXXXXXX\veri_test\c17.v(8): INFO: compiling module 'c17' (VNLR-1012)
F:\XXXXXXXXXXXXXXX\veri_test\c17.v(8): INFO: setting 'c17' as top level module (VNLR-1015)
 ###############################################################
 #      Critical path:
 #      INPUT -> N3, NAND-> N11, NAND-> N16, -> OUTPUT N23
 ###############################################################

 ###############################################################
 #      Gate slacks:
 #      INPUT   -> N6:12.246645 ps
 #      INPUT   -> N3:12.246645 ps
 #      INPUT   -> N1:61.923950 ps
 #      INPUT   -> N2:57.970204 ps
 #      INPUT   -> N7:61.923950 ps
 #      NAND    -> N10:61.923950 ps
 #      OUTPUT  -> N23:12.246645 ps
 #      NAND    -> N11:12.246645 ps
 #      OUTPUT  -> N22:12.246645 ps
 #      NAND    -> N19:20.896521 ps
 #      NAND    -> N16:12.246645 ps
 ###############################################################

 ###############################################################
 #      End of Design summary!
 ###############################################################
```


## Screenshots

Printscreen png files are located in ./doc folder                                                            
![Alt text](/doc/VIP_STA_printscreen.jpg?raw=true "Optional Title")


