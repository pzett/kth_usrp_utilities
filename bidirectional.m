  
load bits_in1
[waveform, parameters]=modem_OFDM4(60,const_size,[1,2],[],1,1,bits_in);
f1=20e6;
f2=20e6;

Y=waveform*3000/sqrt(parameters.power);
Y=[Y,zeros(1,3000-length(Y))];

X=rxtx_bidirectional(40000,Y,f1,f2,25e6/4,100000,2500); 
