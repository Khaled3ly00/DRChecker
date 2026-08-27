rule min_spacing -layer M1 -value 0.25
rule min_spacing -value 0.25 -layer M2
rule min_width -layer M1 -value 0.20
rule min_enclosure -inner VIA1 -outer M1 -value 0.10
rule density -layer M1 -limit minimum -value 0.30 -window_size 10 -window_step 5
rule density -layer M2 -limit maximum -value 0.70 -window_size 20 -window_step 10
rule density -layer M2 -limit maximum -value 0.70 -window_size 20 -window_step 10 -region {0 0 100 100}

#foreach layer_name {M1 M2} {
#   set spacing_value 2
#   rule "min_spacing" \
#        -layer $layer_name \
#        -value $spacing_value
#}