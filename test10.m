


rand('twister',0);
bits_in=rand(1,1856)>0.5;
Ns=60;
[waveform, parameters]=modem_OFDM4(Ns,4,[1,2],1,1,bits_in,[1,2,5:(Ns+2)]);


[hard_bits,h,rx,power,CPECS] = demod_OFDM4(waveform,parameters,144);

