layer M5
layer M5
#rule min_spacing -layer M1 -value
#rule min_spacing -layer M1 -layer M2 -value 0.25
#rule min_spacing -layer M1 -something 5 -value 0.25
#rule density -layer M1 -limit invalid -value 0.30 -window_size 10 -window_step 5
#rule density -layer M1 -limit minimum -value 0.30 -window_size 10 -window_step 5 -region {0 0 100}
#rule min_enclosure -inner CO -options {{}}
#rule min_spacing -layer m1 -value 0.25
#rule min_enclosure -inner CO -options {{-outer nullptr -all_sides 0.04}}
#rule min_spacing -layer M5 -value 0.25