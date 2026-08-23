#rule min_spacing -layer Metal1 -value
#rule min_spacing -layer Metal1 -layer Metal2 -value 0.25
#rule min_spacing -layer Metal1 -something 5 -value 0.25
#rule density -layer Metal1 -limit invalid -value 0.30 -window_size 10 -window_step 5
rule density -layer Metal1 -limit minimum -value 0.30 -window_size 10 -window_step 5 -region {0 0 100}