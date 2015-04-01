
function [BER1,BER2,BER1i,BER2i]=process_MIMO_measurement(X,twister_seed,MMSE, part)
%
% Process the MIMO measurements.
% X: The received signal.
% twister_seed: The seed used by the transmitter when generating bits (necesary for BER calculation).
% MMSE: If MMSE=1, then the MMSE receiver is used otherwise maximum ratio combining is used.
% part: If part=0, then the signals directly following the synchronization sequence is used.
%       If part=1, then the second set of signals after the synchronization sequency is used.
% BER1, BER2: The BER of stream one and two, respectively.
% BER1i, BER2i. The BER of stream one and two without cross-talk between the MIMO channels ("i"=ideal).
%
%


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

% Remmove freqeuency offset from the data
X(1,:)=X(1,:).*exp(-j*2*pi*f_offset*(1:length(X)));
X(2,:)=X(2,:).*exp(-j*2*pi*f_offset*(1:length(X)));

%[ start_pos, f_offset_UNUSED] = synchronize_OFDM1(X(1,1:10000), parameters1,0,1, 1);


% Positioning of demodulation to remove cyclic prefix. 
offset=3;

% Demodulate first part of the data 
[hard_bits1,h1,rx1,power] = demod_OFDM4(X, parameters1,start_pos+141+offset+part*9000,MMSE);
[hard_bits2,h2,rx2,power] = demod_OFDM4(X, parameters2,start_pos+141+offset+part*9000,MMSE);


BER1=1-mean(hard_bits1'==bits_in1);
BER2=1-mean(hard_bits2'==bits_in2);


% Signal from TX antenna 1 only
X1=X(:,part*9000+(start_pos+3+N)+(0:N));
% Signal from TX antenna 2 only
X2=X(:,part*9000+(start_pos+3+N*2)+(0:N));


% Signal from TX antenna 1 only
[hard_bits1,h1,rx1,power] = demod_OFDM4(X, parameters1,start_pos+offset+N,MMSE);
% Signal from TX antenna 2 only
[hard_bits2,h2,rx2,power] = demod_OFDM4(X, parameters2,start_pos+offset+2*N,MMSE);

BER1i=1-mean(hard_bits1'==bits_in1);
BER2i=1-mean(hard_bits2'==bits_in2);


