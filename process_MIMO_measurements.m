% process_MIMO_measurements.m
%
% The measurements were performed running demo_MIMO_tx.m in the transmitter with
% parameters use_50Msps=0 rf_freq=60e9; repeat_single_antenna=1; 
% const_size=4; gain_tx=10 and varying twister_seed. The transmittded signal
% has the form: 
%
% synchronization_sequence -> both antennas transmitting (MIMO) ->antenna 1 only ->antenna 2 only.
%
% This patterns is repeated until the measurement stops. The total length of the
% sequency is 9000 samples. At the receiver 30000 samples are captured at a time
% using the script log_data.m. Thus
% at least two complete transmit patterns should be received.
% The function process_MIMO_measurement.m processes on measurement file.
% It first searches for the first occurance of a synzhronization sequence in the data.
% then it picks out the different parts of the data base on that position.
% The MIMO transmission is located 144 samples after the sync, the "transmitter antenna
% one only" part is located 2869 samples after the sync and "transmitter antena two
% only" part is located 5738 samples after the sync. The sequence repeated every 
% 9000 samples. Thus by stepping forward an additional 9000 samples a new replica
% of the transmitted sequencies are obtained. This correspond to the "part" input
% parameter of process_MIMO_measurement.


% The measurements
% 1-12. Horizontal orientation of receiver antennas. 
% 101-111 Horizontal orientation of receiver antennas. 
% 201-209 Horizontal orientation of receiver antennas. 
% 301-311 Horizontal orientation of receiver antennas. 
% 401-411 Horizontal orientation of receiver antennas. 
% 501-510 Horizontal orientation of receiver antennas. 
% 601-608 Tilted orientation of receiver antennas. 
% 701-710 Tilted orientation of receiver antennas. 
% 801-810 Tilted orientation of receiver antennas. 
% 901-910 Tilted orientation of receiver antennas. 
% 1001-1009 Tilted orientation of receiver antennas. 


fprintf(1,'Download the MIMO measurements. \n');
fprintf(1,'Then make sure meas_dir points to the location of the measurements. \n');
meas_dir0='/home/perz/Dropbox/open_source_60GHz/MIMO_signals/';

if 1
i10=1;
for i1=[1:12,101:111,201:209,301:311,401:411,501:510,601:608,701:710,801:810,901:909,1001:1009]
 
    i1
    

    % Load the i1:th measurement. The files contain the matrix X which is of size 2x20000
    % where row one contains the signal received on antenna one and row two the signal
    % received on antenna two. The files also contains the variable twister_seed which
    % is used to initialize the random number generator in the transmitter.

    filename=['MIMO_meas',num2str(i1)];
    cmd_str=['load ',meas_dir0,filename];    
    eval(cmd_str);

       
    MMSE=1;
    [BER1,BER2,BER1i,BER2i]=process_MIMO_measurement(X,twister_seed,MMSE,0);

    
    BER1s_MMSE(i10)=BER1;  % Actual BER
    BER2s_MMSE(i10)=BER2;
    BER1si_MMSE(i10)=BER1i; % Without interference
    BER2si_MMSE(i10)=BER2i;
    
    MMSE=0; %% Maximum ratio combining
    [BER1,BER2,BER1i,BER2i]=process_MIMO_measurement(X,twister_seed,MMSE,0);
    [BER1,BER2,BER1i,BER2i]
    
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
end;

close

load MIMO_res
Fig=gcf;

set(Fig,'Color',[1 1 1]);
BER=[BER1s,BER2s];
subplot(311)
hist(BER,20)
grid
axis([0 0.6 0 170]);
BER_MMSE=[BER1s_MMSE,BER2s_MMSE];
subplot(312)
hist(BER_MMSE,20)
grid
axis([0 0.6 0 170]);
subplot(313)
BERi=[BER1si,BER2si];
hist(BERi,20);
grid
axis([0 0.6 0 170]);
xlabel('BER');

'print -deps BER_hist.eps'
