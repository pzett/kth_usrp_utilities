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
% along with this program.  If not, see <http://www.gnu.org/licenses/>.
%


%===================================================
% Set these parameters to obtain desired behaviour 
%===================================================
usrp_addr='192.168.20.2'; 
use_50Msps=0;
gain_tx=13;
rf_freq=60e9;
const_size=4; %% 4=QPSK, 16=16QAM, 64=64QAM
%===================================================


rand('twister',0);
%bits_in=rand(1,1888)>0.5;
bits_in=rand(1,1856*round(log2(const_size)/2))>0.5;
[waveform, parameters]=modem_OFDM3(60,4,1,1,1,bits_in);


if (use_50Msps)
  rate=50e6;
  low_res=1;
else
  rate=25e6;
  low_res=0;
end;


tx_60GHz(rf_freq, 3000, waveform*3000/sqrt(parameters.power),0, gain_tx, ...
rate, low_res, usrp_addr);
