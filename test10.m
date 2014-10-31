

rand('twister',0);
bits_in=rand(1,118)>0.5;
[waveform, parameters]=modem_OFDM4(60,4,1,[1,2],1,bits_in,[],10,2,1:3);

