



gain_rx=14;
%rand('twister',0);
%bits_in=rand(1,1888)>0.5;
load bits_in
[waveform, parameters]=modem_OFDM3(60,4,1,1,1,bits_in);
%tx_60GHz(5000, waveform*5000/sqrt(parameters.power),0, 13, 25e6);
scale=0.2;


X=rx_60GHz(10000,0,gain_rx,25e6,0);
%X=rx_60GHz(10000,0,gain_rx,50e6,0,scale);

[ start_pos, f_offset] = synchronize_OFDM1(X(1:5000), parameters,1,1, 1);
X=X.*exp(-j*2*pi*f_offset*(1:length(X)));

while (1)

  [hard_bits,h,rx,power,CPECS] = demod_OFDM3(X, parameters,start_pos+144);
  figure(1);
  plot(rx(:),'x');
  BER=1-mean(hard_bits'==bits_in);
  text(-1.8,1.8,['BER=',num2str(round(BER*1000)/10),'%'],'FontSize',48);
  axis([-2 2 -2 2]);
  e=evm(rx(:),parameters.tx(:));
  text(1.0,1.8,['EVM=',num2str(round(e*10)/10),'%'],'FontSize',48);
  title('60GHz transmission');
  figure(2);  
  plot(abs(X))
  title('60GHz transmission','FontSize',48);
  pause(1);
  X=rx_60GHz(10000,0,gain_rx,25e6,0);
  %X=rx_60GHz(10000,0,gain_rx,50e6,0,scale);
  scale=max(abs(X))/2^15*2;
  X=X.*exp(-j*2*pi*f_offset*(1:length(X)));
  [ start_pos, f_offsett] = synchronize_OFDM1(X(1:5000), parameters,0,1, 1);
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
