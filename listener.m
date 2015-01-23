twister_receive=14;
const_size=4;
load bits_in2
%rand('twister',twister_receive);
%bits_in=rand(1,1856*round(log2(const_size)/2))>0.5;
[waveform, parameters]=modem_OFDM4(60,const_size,[1,2],[],1,1,bits_in);






t = tcpip('127.0.0.1', 30000, 'NetworkRole', 'server');
fopen(t);
buffer_size=10000;
X=zeros(1,buffer_size);


while 1
while (t.BytesAvailable==0)
    pause(0.1);
end;
N=buffer_size;
i1=0;
while (t.BytesAvailable) & (N>0)
    n=t.BytesAvailable/2;
    i3=min(N,n);
    ix=((1:i3)+i1)';
    X(ix)=fread(t,i3,'int16')';
    N=N-n;
    i1=i1+n;
    pause(0.1);
end; 

N=buffer_size;
i1=0;
while (t.BytesAvailable) & (N>0)
    n=t.BytesAvailable/2;
    i3=min(N,n);
    ix=((1:i3)+i1)';
    X(ix)=X(ix)+j*fread(t,i3,'int16')';
    N=N-n;
    i1=i1+n;
    pause(0.1);
end; 



  [ start_pos, f_offset] = synchronize_OFDM1(X(1:5000), parameters,0,1, 1);
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

end;
