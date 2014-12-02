% log_data.m
meas_dir='/home/eq2430/meas141128/';

for i1=301:400    
    i1
    c=clock;
    filename=['MIMO_meas',num2str(i1)];
    X=rx_60GHz_MIMO(30000,gain_rx,rate,scaling_8bits);
    cmd_str=['save ',meas_dir,filename,' X twister_seed'];
    
    eval(cmd_str);
    
end;


% Walkning away from transmitter
% 1-12. Horizontal orientation of receiver antennas. twister_seed=0
% 101-112 Horizontal orientation of receiver antennas. twister_seed=1
% 201-209 Horizontal orientation of receiver antennas. twister_seed=2
% 301-311 Horizontal orientation of receiver antennas. twister_seed=3
% 401-411 Horizontal orientation of receiver antennas. twister_seed=4
% 501-511 Horizontal orientation of receiver antennas. twister_seed=5
% 601-611 Tilted orientation of receiver antennas. twister_seed=6
% 701-710 Tilted orientation of receiver antennas. twister_seed=7
% 801-810 Tilted orientation of receiver antennas. twister_seed=8
% 901-910 Tilted orientation of receiver antennas. twister_seed=9
% 1001-1009 Tilted orientation of receiver antennas. twister_seed=10