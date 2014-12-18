  
%===================================================
% Set these parameters to obtain desired behaviour 
%===================================================
usrp_addr='192.168.10.2';
use_50Msps=0;
const_size=4;
gain_rx=14;
rf_freq=60e9;
const_size=4; %% 4=QPSK, 16=16QAM, 64=64QAM
%===================================================

if (use_50Msps)
  rate=50e6;
  scaling_8bits=-2;
else
  rate=25e6;
  scaling_8bits=0;
end;
 

rand('twister',0);
bits_in=rand(1,1856*round(log2(const_size)/2))>0.5;
[waveform, parameters]=modem_OFDM4(60,const_size,[1,2],[],1,1,bits_in);


X=rx_60GHz(rf_freq,10000,0,gain_rx,rate,scaling_8bits);

[ start_pos, f_offset] = synchronize_OFDM1(X(1:5000), parameters,1,1, 1);

X=X.*exp(-j*2*pi*f_offset*(1:length(X)));


while (1)

  [hard_bits,h,rx,power] = demod_OFDM4(X, parameters,start_pos+144);

  figure(1);
  plot(rx(:),'x');
  BER=1-mean(hard_bits'==bits_in);
  text(-1.8,1.8,['BER=',num2str(round(BER*1000)/10),'%'],'FontSize',48);
  axis([-2 2 -2 2]);
  e=evm(rx(:),parameters.tx(:));
  text(1.0,1.8,['EVM=',num2str(round(e*10)/10),'%'],'FontSize',48);
  title('60GHz transmission');
  figure(2);  
  plot(abs(X(10:end)))
  title('60GHz transmission','FontSize',48);
  pause(1);

		
  
  X=rx_60GHz(rf_freq,10000,0,gain_rx,rate,scaling_8bits);  
  X=X.*exp(-j*2*pi*f_offset*(1:length(X))); 


  % Use the frequency offset that was computed the first time
  estimate_freq_offset = 0; % Do not estimate frequency offset 
		            % in the following steps
  [ start_pos, f_offset_UNUSED] = synchronize_OFDM1(X(1:5000), parameters,estimate_freq_offset,1, 1); 


end;

