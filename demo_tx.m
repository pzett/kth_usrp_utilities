




rand('twister',0);
bits_in=rand(1,1888)>0.5;
[waveform, parameters]=modem_OFDM3(60,4,1,1,1,bits_in);


tx_60GHz(3000, waveform*5000/sqrt(parameters.power),0, 13, 25e6);
%tx_60GHz(3000, waveform*5000/sqrt(parameters.power),0, 13, 25e6, 0, 0, "192.168.20.2");

%tx_60GHz(3000, waveform*5000/sqrt(parameters.power),0, 13, 50e6, 0, 1, "192.168.20.2");

