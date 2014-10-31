
usrp_addr='192.168.10.2';
use_50Msps=0;


rf_freq=60e9;

rand('twister',0);
bits_in=rand(1,1856)>0.5;
Ns=60;
[waveform, parameters]=modem_OFDM4(Ns,4,[1,2],1,1,bits_in,[1,2,5:(Ns+2)]);



if (use_50Msps)
  rate=50e6;
  low_res=1;
else
  rate=25e6;
  low_res=0;
end;


tx_60GHz(rf_freq, 3000, waveform*5000/sqrt(parameters.power),0, 10, ...
rate, low_res, usrp_addr);
