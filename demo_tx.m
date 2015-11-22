

%===================================================
% Set these parameters to obtain desired behaviour 
%===================================================
usrp_addr='192.168.20.2'; 
use_50Msps=0;
gain_tx=13;
rf_freq=64e9;
const_size=4; %% 4=QPSK, 16=16QAM, 64=64QAM
BasicDB_freq=15e6;
%===================================================
% 285.71385

rand('twister',0);
bits_in=rand(1,1856*round(log2(const_size)/2))>0.5;
[waveform, parameters]=modem_OFDM4(60,const_size,[1,2],[],1,1,bits_in);


if (use_50Msps)
  rate=50e6;
  low_res=1;
else
  rate=25e6;
  low_res=0;
end;


tx_60GHz(rf_freq, 3000, waveform*3000/sqrt(parameters.power),0, gain_tx, ...
	 rate, low_res, usrp_addr,BasicDB_freq);
