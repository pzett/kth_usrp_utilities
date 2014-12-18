
%===================================================
% Set these parameters to obtain desired behaviour 
%===================================================
% Addresses of USRPs assumed to be 192.168.10.2 and 192.168.20.2.
use_50Msps=0;
const_size=4; %% Constellation size. Use 4 for QPSK and 16 for 16QAM.
twister_seed=0;
rf_freq=60e9;
gain_rx=10;
%===================================================

if (use_50Msps)
  rate=50e6;
  scaling_8bits=-2;
else
  rate=25e6;
  scaling_8bits=0;
end;
 

rand('twister',twister_seed);
bits_in1=rand(1,1740*round(log2(const_size)/2))>0.5;
bits_in2=rand(1,1740*round(log2(const_size)/2))>0.5;
Ns=60;
[waveform1, parameters1]=modem_OFDM4(Ns,const_size,[1,2],[3,4],[1,2],1,bits_in1,[1,2,5:(Ns+2)]);
[waveform2, parameters2]=modem_OFDM4(Ns,const_size,[1,2],[1,2],[2,1],2,bits_in2,[3:(Ns+2)]);

X=rx_60GHz_MIMO(10000,gain_rx,rate,scaling_8bits);
[ start_pos, f_offset] = synchronize_OFDM1(X(1,1:5000), parameters1,1,1, 1);

X(1,:)=X(1,:).*exp(-j*2*pi*f_offset*(1:length(X)));
X(2,:)=X(2,:).*exp(-j*2*pi*f_offset*(1:length(X)));


while (1)

  [hard_bits1,h1,rx1,power] = demod_OFDM4(X, parameters1,start_pos+144);
  [hard_bits2,h2,rx2,power] = demod_OFDM4(X, parameters2,start_pos+144);

  figure(1);
  subplot(211);
  plot(rx1(:),'x');
  subplot(212);
  plot(rx2(:),'x');

  BER=1-mean(hard_bits1'==bits_in1);
  text(-1.8,1.8,['BER=',num2str(round(BER*1000)/10),'%'],'FontSize',48);
  axis([-2 2 -2 2]);
  e=evm(rx1(:),parameters1.tx(:));
  text(1.0,1.8,['EVM=',num2str(round(e*10)/10),'%'],'FontSize',48);
  title('60GHz transmission');
  figure(2);  
  plot(abs(X(:,100:end)'))
  title('60GHz transmission','FontSize',48);

  pause(1);


  % Use the frequency offset that was computed the first time
  estimate_freq_offset = 0; % Do not estimate frequency offset 
		            % in the following steps


  X=rx_60GHz_MIMO(10000,gain_rx,rate,scaling_8bits);

  X(1,:)=X(1,:).*exp(-j*2*pi*f_offset*(1:length(X)));
  X(2,:)=X(2,:).*exp(-j*2*pi*f_offset*(1:length(X)));
  [ start_pos, f_offset_UNUSED] = synchronize_OFDM1(X(1,1:5000), parameters1,estimate_freq_offset,1, 1);





end;


if 0

  while 1
  X=rx_60GHz(10000,0,14,25e6,0); 
  hold off
  figure(3)
  plot(10*log10(abs(conv(abs(X).^2,ones(1,100)))),'r')
  hold on
  plot([0 10000],[35 35]);
  plot([0 10000],[80 80]);
  axis([0 10000 10 100]);
  pause(0.1);
  end;



end;
