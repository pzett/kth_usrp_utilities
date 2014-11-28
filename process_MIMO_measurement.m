

% Data is located in "X"

% Obtain modulation parameters
rand('twister',twister_seed);
bits_in1=rand(1,1740)>0.5;
bits_in2=rand(1,1740)>0.5;
Ns=60;
[waveform1, parameters1]=modem_OFDM4(Ns,4,[1,2],[3,4],[1,2],2,bits_in1,[1,2,5:(Ns+2)]);
[waveform2, parameters2]=modem_OFDM4(Ns,4,[1,2],[1,2],[2,1],1,bits_in2,[3:(Ns+2)]);
N=length(waveform1);

% Frequency and time synchronization 
[ start_pos, f_offset] = synchronize_OFDM1(X(1,1:10000), parameters1,1,1, 1);
X(1,:)=X(1,:).*exp(-j*2*pi*f_offset*(1:length(X)));
X(2,:)=X(2,:).*exp(-j*2*pi*f_offset*(1:length(X)));

% Demodulate first part of the data 
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
text(1.0,1.8,['EVM=',num2str(round(e*10)/10),'%'],'FontSize',48);
title('60GHz transmission');
figure(2);  
plot(abs(X(:,100:end)'))
title('60GHz transmission','FontSize',48);

keyboard

pause(1);


% Demodulate the second part of the data 
[hard_bits1,h1,rx1,power] = demod_OFDM4(X, parameters1,start_pos+3+N);
[hard_bits2,h2,rx2,power] = demod_OFDM4(X, parameters2,start_pos+3+N*2);

figure(1);
subplot(211);
plot(rx1(:),'x');
subplot(212);
plot(rx2(:),'x');

BER=1-mean(hard_bits1'==bits_in1);
text(-1.8,1.8,['BER=',num2str(round(BER*1000)/10),'%'],'FontSize',48);
axis([-2 2 -2 2]);
text(1.0,1.8,['EVM=',num2str(round(e*10)/10),'%'],'FontSize',48);
title('60GHz transmission');
figure(2);  
plot(abs(X(:,100:end)'))
title('60GHz transmission','FontSize',48);
pause(1);
