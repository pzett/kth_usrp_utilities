
usrp_addr='192.168.10.2';
use_50Msps=0;


rf_freq=60e9;

rand('twister',0);
bits_in=rand(1,118)>0.5;
[waveform, parameters]=modem_OFDM4(60,4,1,[1,2],1,bits_in,[],10,2,1:3);


if (use_50Msps)
  rate=50e6;
  low_res=1;
else
  rate=25e6;
  low_res=0;
end;


tx_60GHz(rf_freq, 3000, waveform*5000/sqrt(parameters.power),0, 10, ...
rate, low_res, usrp_addr);
