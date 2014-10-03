
usrp_addr='192.168.20.2';
use_50Msps=0;


rf_freq=60e9;
rand('twister',0);
bits_in=rand(1,1888)>0.5;
[waveform, parameters]=modem_OFDM3(60,4,1,1,1,bits_in);


if (use_50Msps)
  rate=50e6;
  low_res=1;
else
  rate=25e6;
  low_res=0;
end;


tx_60GHz(rf_freq, 3000, waveform*5000/sqrt(parameters.power),0, 13, ...
rate, low_res, usrp_addr);
