
usrp_addr='192.168.10.2';
use_50Msps=0;


rf_freq=60e9;

rand('twister',0);
bits_in1=rand(1,1740)>0.5;
bits_in2=rand(1,1740)>0.5;
Ns=60;
[waveform1, parameters1]=modem_OFDM4(Ns,4,[1,2],[3,4],[1,2],1,bits_in1,[1,2,5:(Ns+2)]);
[waveform2, parameters2]=modem_OFDM4(Ns,4,[3,4],[1,2],[2,1],2,bits_in2,[3:(Ns+2)]);

X=zeros(2,length(waveform1));
X(1,:)= waveform1*5000/sqrt(parameters1.power);
X(2,:)= waveform2*5000/sqrt(parameters2.power);

if (use_50Msps)
  rate=50e6;
  low_res=1;
else
  rate=25e6;
  low_res=0;
end;


tx_60GHz_MIMO(3000, X, 10, rate, low_res)
