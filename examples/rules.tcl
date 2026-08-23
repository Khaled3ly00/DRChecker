rule min_spacing -layer Metal1 -value 0.25
rule min_spacing -value 0.25 -layer Metal2
rule min_width -layer Metal1 -value 0.20
rule min_enclosure -inner Via12 -outer Metal1 -value 0.10
rule density -layer Metal1 -limit minimum -value 0.30 -window_size 10 -window_step 5
rule density -layer Metal2 -limit maximum -value 0.70 -window_size 20 -window_step 10
rule density -layer Metal2 -limit maximum -value 0.70 -window_size 20 -window_step 10 -region {0 0 100 100}

#foreach layer_name {Metal1 Metal2} {
#   set spacing_value 2
#   rule "min_spacing" \
#        -layer $layer_name \
#        -value $spacing_value
#}