
%
% Copyright 2014 Modified by Per Zetterberg, KTH.
%
% This program is free software: you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation, either version 3 of the License, or
% (at your option) any later version.
%
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with this program.  If not, see <http:%www.gnu.org/licenses/>.
%


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

%[ start_pos, f_offset_UNUSED] = synchronize_OFDM1(X(1,1:10000), parameters1,0,1, 1);

% Demodulate first part of the data 
[hard_bits1,h1,rx1,power] = demod_OFDM4(X, parameters1,start_pos+144,MMSE);
[hard_bits2,h2,rx2,power] = demod_OFDM4(X, parameters2,start_pos+144,MMSE);



BER1=1-mean(hard_bits1'==bits_in1)
BER2=1-mean(hard_bits2'==bits_in2)



% Signal from TX antenna 1 only
X1=X(:,0*9000+(start_pos+3+N)+(0:N));
% Signal from TX antenna 2 only
X2=X(:,0*9000+(start_pos+3+N*2)+(0:N));


% Demodulate the second part of the data 
[hard_bits1,h1,rx1,power] = demod_OFDM4(X1, parameters1,1,MMSE);
[hard_bits2,h2,rx2,power] = demod_OFDM4(X2, parameters2,1,MMSE);


BER1i=1-mean(hard_bits1'==bits_in1)
BER2i=1-mean(hard_bits2'==bits_in2)


