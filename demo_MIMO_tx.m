
use_50Msps=0;
rf_freq=60e9;
repeat_single_antenna=0; % Used when logging data for post-processing at the receiver.
const_size=4; %% Use 4 or 16 for QPSK and 16QAM
twister_seed=0;

rand('twister',twister_seed);
bits_in1=rand(1,1740*round(log2(const_size)/2))>0.5;
bits_in2=rand(1,1740*round(log2(const_size)/2))>0.5;
Ns=60;

% Waveforms modulated by different bits. Only antenna 2 transmits sync.
[waveform1, parameters1]=modem_OFDM4(Ns,const_size,[1,2],[3,4],[1,2],2,bits_in1,[1,2,5:(Ns+2)]);
[waveform2, parameters2]=modem_OFDM4(Ns,const_size,[1,2],[1,2],[2,1],1,bits_in2,[3:(Ns+2)]);
N=length(waveform1);

X=zeros(2,N+2*N*repeat_single_antenna);
X(1,1:N)= waveform1*2000/sqrt(parameters1.power);
X(2,1:N)= waveform2*2000/sqrt(parameters2.power);


if repeat_single_antenna

  % Repeat the two waveforms with only one antenna active at a time. No sync.
  [waveform1, parameters1]=modem_OFDM4(Ns,4,[1,2],[3,4],[1,2],0,bits_in1,[1,2,5:(Ns+2)]);
  [waveform2, parameters2]=modem_OFDM4(Ns,4,[1,2],[1,2],[2,1],0,bits_in2,[3:(Ns+2)]);

  X(1,(N+1):(N+length(waveform1)))= waveform1*2000/sqrt(parameters1.power);
  X(2,(2*N+1):(2*N+length(waveform2)))= waveform2*2000/sqrt(parameters2.power);

end;


if (use_50Msps)
  rate=50e6;
  low_res=1;
else
  rate=25e6;
  low_res=0;
end;


tx_60GHz_MIMO(ceil(size(X,2)/1000)*1000, X, 10, rate, low_res)
