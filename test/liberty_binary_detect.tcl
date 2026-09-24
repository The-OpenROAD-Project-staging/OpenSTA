# A binary liberty file is recognized by its contents, not only by the
# .blib extension.
source helpers.tcl

set renamed_file [make_result_file liberty_binary_detect.lib]
write_liberty_binary liberty_float_as_str.lib $renamed_file
read_liberty $renamed_file
report_units
puts "cells: [llength [get_lib_cells *]]"
