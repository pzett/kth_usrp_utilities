




rand('twister',0);
bits_in=rand(1,1888)>0.5;
[waveform, parameters]=modem_OFDM3(60,4,1,1,1,bits_in);


tx_60GHz(5000, waveform*5000/sqrt(parameters.power),0, 13, 25e6);
