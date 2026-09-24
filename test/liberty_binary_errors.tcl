# Error handling for binary liberty files.
source helpers.tcl

# Report an error message with the results directory stripped.
proc puts_error { msg } {
  global result_dir
  puts [string map [list "$result_dir/" ""] $msg]
}

# A text liberty file with a .blib extension is not binary liberty.
set fake_blib [make_result_file liberty_binary_errors_text.blib]
file copy -force liberty_float_as_str.lib $fake_blib
catch { read_liberty $fake_blib } result
puts_error $result

# A truncated .blib is rejected rather than partially loaded.
set blib_file [make_result_file liberty_binary_errors.blib]
write_liberty_binary liberty_float_as_str.lib $blib_file
set stream [open $blib_file r+]
chan truncate $stream [expr { [file size $blib_file] / 2 }]
close $stream
catch { read_liberty $blib_file } result
puts_error $result
puts "libraries loaded: [llength [get_libs -quiet *]]"

# A .blib written by another format version is rejected with its version.
set old_blib [make_result_file liberty_binary_errors_v1.blib]
write_liberty_binary liberty_float_as_str.lib $old_blib
set stream [open $old_blib r+]
fconfigure $stream -translation binary
seek $stream 8
puts -nonewline $stream [binary format iu 1]
close $stream
catch { read_liberty $old_blib } result
puts_error $result

# A missing .blib reports the file, not a corrupt file.
catch { read_liberty [make_result_file does_not_exist.blib] } result
puts_error $result

# A missing input leaves no partial output behind.
set out_file [make_result_file liberty_binary_errors_missing_input.blib]
catch { write_liberty_binary does_not_exist.lib $out_file } result
puts_error $result
puts "partial output left behind: [file exists $out_file]"

# A syntax error in the input leaves no partial output behind either.
set bad_lib [make_result_file liberty_binary_errors_truncated.lib]
file copy -force liberty_float_as_str.lib $bad_lib
set stream [open $bad_lib r+]
chan truncate $stream [expr { [file size $bad_lib] / 2 }]
close $stream
set out_file [make_result_file liberty_binary_errors_truncated.blib]
catch { write_liberty_binary $bad_lib $out_file } result
puts_error $result
puts "partial output left behind: [file exists $out_file]"

# An unwritable output path is reported.
catch { write_liberty_binary liberty_float_as_str.lib /nonexistent_dir/out.blib } result
puts_error $result
