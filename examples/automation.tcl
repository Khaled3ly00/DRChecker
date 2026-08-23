# SCRIPT WORKING DIRECTORY
set script_dir [file dirname [file normalize [info script]]]

set layout_file "$script_dir/cli_multiple_shapes_layout.json"
set rules_file "$script_dir/equivalent_rules.tcl"
set report_file "$script_dir/automation_report.json"

drc_run -layout $layout_file \
        -rules $rules_file \
        -report $report_file

set error_count [drc_error_count]

if {[drc_error_count] > 0} {
    puts "DRC found [drc_error_count] violations"
} else {
    puts "DRC passed with no violations"
}