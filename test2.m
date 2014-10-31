

Nsamples=20000;
Ns=100;
X=zeros(2,Ns*21);
gain=10;
tx_rate=50e6;
low_res=1;
rand('twister',0);
bits_in=rand(1,1888)>0.5;
[waveform, parameters]=modem_OFDM3(60,4,1,1,1,bits_in);


for i1=1:10
   ix=(1:Ns)+(i1-1)*2*Ns;
   X(1,ix)=5000*ones(1,Ns); 
   ix=ix+Ns;
   a=(i1-1)*20;
   X(2,ix)=5000*exp(j*pi/180*a)*ones(1,Ns);
end;

train=waveform(1:140)*20000;
X=[[train;zeros(size(train))],X];

tx_60GHz_MIMO(size(X,2), X, gain, tx_rate, low_res);
