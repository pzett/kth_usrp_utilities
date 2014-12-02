
% process_MIMO_measurements.m

% The measurements
% 1-12. Horizontal orientation of receiver antennas. twister_seed=0
% 101-111 Horizontal orientation of receiver antennas. twister_seed=1
% 201-209 Horizontal orientation of receiver antennas. twister_seed=2
% 301-311 Horizontal orientation of receiver antennas. twister_seed=3. Ta
% om.
% 401-411 Horizontal orientation of receiver antennas. twister_seed=4
% 501-510 Horizontal orientation of receiver antennas. twister_seed=5
% 601-608 Tilted orientation of receiver antennas. twister_seed=6
% 701-710 Tilted orientation of receiver antennas. twister_seed=7
% 801-810 Tilted orientation of receiver antennas. twister_seed=8
% 901-910 Tilted orientation of receiver antennas. twister_seed=9
% 1001-1009 Tilted orientation of receiver antennas. twister_seed=10

% Point to location of the measurements
meas_dir='/home/eq2430/meas141128/';

i10=1;
for i1=[1:12,101:111,201:209,301:311,401:411,501:510,601:608,701:710,801:810,901:909,1001:1009]
 
    i1
    
    filename=['MIMO_meas',num2str(i1)];
    cmd_str=['load ',meas_dir,filename];    
    eval(cmd_str);
       
    MMSE=1;
    process_MIMO_measurement;
    
    BER1s_MMSE(i10)=BER1;  % Actual BER
    BER2s_MMSE(i10)=BER2;
    BER1si_MMSE(i10)=BER1i; % Without interference
    BER2si_MMSE(i10)=BER2i;
    
    MMSE=0; %% Maximum ratio combining
    process_MIMO_measurement;    
    BER1s(i10)=BER1;  % Actual BER
    BER2s(i10)=BER2;
    BER1si(i10)=BER1i; % Without interference
    BER2si(i10)=BER2i;
    
    
    
    i10=i10+1;
       

    hold on
    plot(BER1s,'-x')
    hold on
    plot(BER1si,'-xg')
    pause(1);
    
end;

save MIMO_res BER1s BER2s BER1s_MMSE BER2s_MMSE BER1si BER2si BER1si_MMSE BER2si_MMSE
