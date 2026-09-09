# IO delays that exist only in a corner overlay must reach the engine's
# gates: a corner-only set_output_delay creates a path endpoint, a
# corner-only set_input_delay on an internal pin seeds a segment start,
# and a bundle redefine with data empties the corner's overlays.
source ../../test/helpers.tcl
read_liberty ../../test/nangate45/Nangate45_slow.lib
read_verilog search_crpr.v
link_design search_crpr

create_clock -name clk -period 10 [get_ports clk]
set_input_delay 1.0 -clock clk [get_ports in1]
set_input_delay 1.0 -clock clk [get_ports in2]
# The mode has no set_output_delay on out1 and no internal input delays.

set io_sdc [make_result_file corner_io_gates.sdc]
set stream [open $io_sdc "w"]
puts $stream {set_output_delay 2.0 -clock clk [get_ports out1]}
puts $stream {set_input_delay 1.5 -clock clk [get_pins buf1/Z]}
close $stream
define_analysis_corner ssc -liberty NangateOpenCellLibrary_slow -sdc [list $io_sdc]
define_scene ss -analysis_corner ssc

puts "=== corner-only output delay endpoint ==="
report_checks -to [get_ports out1] -format end
puts "=== corner-only internal input delay segment start ==="
report_checks -from [get_pins buf1/Z] -format end

# Redefine with data replaces the bundle and empties the overlay: the
# corner-only constraints are gone.
define_analysis_corner ssc -liberty NangateOpenCellLibrary_slow
puts "=== after redefine with data ==="
report_checks -to [get_ports out1] -format end
report_checks -from [get_pins buf1/Z] -format end

# Overlay references die with their objects. delete_clock purges the
# corner's constraints referencing the clock (in2 falls back to the
# mode's clk delay); delete_instance purges pin-keyed corner constraints.
create_clock -name vclk -period 10
set_cmd_analysis_corner ssc
set_input_delay 2.0 -clock vclk [get_ports in2]
set_clock_latency -source 0.4 [get_clocks vclk]
unset_cmd_analysis_corner
puts "=== corner delay on in2 wrt vclk ==="
report_checks -from [get_ports in2] -format end
delete_clock vclk
puts "=== after delete_clock vclk: falls back to mode delay ==="
report_checks -from [get_ports in2] -format end

set_cmd_analysis_corner ssc
set_input_delay 1.2 -clock clk [get_pins buf2/Z]
unset_cmd_analysis_corner
delete_instance buf2
puts "=== after delete_instance buf2 ==="
report_checks -format end
