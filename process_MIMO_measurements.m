
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


% process_MIMO_measurements.m

% The measurements
% 1-12. Horizontal orientation of receiver antennas. twister_seed=0
% 101-111 Horizontal orientation of receiver antennas. twister_seed=1
% 201-209 Horizontal orientation of receiver antennas. twister_seed=2
% 301-311 Horizontal orientation of receiver antennas. twister_seed=3. Ta
% om.
% 401-411 Horizontal orientation of receiver antennas. twister_seed=4
% 501-510 Horizontal orientation of receiver antennas. twister_seed=5
% 601-608 Tilted orientation of receiver antennas. twister_seed=6
% 701-710 Tilted orientation of receiver antennas. twister_seed=7
% 801-810 Tilted orientation of receiver antennas. twister_seed=8
% 901-910 Tilted orientation of receiver antennas. twister_seed=9
% 1001-1009 Tilted orientation of receiver antennas. twister_seed=10


fprintf(1,'Download the measurements. \n');
fprintf(1,'Then make sure meas_dir points to the location of the measurements. \n');
meas_dir='/home/eq2430/meas141128/';

if 0
i10=1;
for i1=[1:12,101:111,201:209,301:311,401:411,501:510,601:608,701:710,801:810,901:909,1001:1009]
 
    i1
    
    filename=['MIMO_meas',num2str(i1)];
    cmd_str=['load ',meas_dir,filename];    
    eval(cmd_str);
       
    MMSE=1;
    process_MIMO_measurement;
    
    BER1s_MMSE(i10)=BER1;  % Actual BER
    BER2s_MMSE(i10)=BER2;
    BER1si_MMSE(i10)=BER1i; % Without interference
    BER2si_MMSE(i10)=BER2i;
    
    MMSE=0; %% Maximum ratio combining
    process_MIMO_measurement;    
    BER1s(i10)=BER1;  % Actual BER
    BER2s(i10)=BER2;
    BER1si(i10)=BER1i; % Without interference
    BER2si(i10)=BER2i;
    
    
    
    i10=i10+1;
       

    hold on
    plot(BER1s,'-x')
    hold on
    plot(BER1si,'-xg')
    pause(1);
    
end;

save MIMO_res BER1s BER2s BER1s_MMSE BER2s_MMSE BER1si BER2si BER1si_MMSE BER2si_MMSE
end;

load MIMO_res
Fig=gcf;

set(Fig,'Color',[1 1 1]);
BER=[BER1s,BER2s];
subplot(311)
hist(BER,20)
grid
axis([0 0.6 0 170]);
BER_MMSE=[BER1s_MMSE,BER2s_MMSE];
subplot(312)
hist(BER_MMSE,20)
grid
axis([0 0.6 0 170]);
subplot(313)
BERi=[BER1si,BER2si];
hist(BERi,20);
grid
axis([0 0.6 0 170]);
xlabel('BER');

'print -deps BER_hist.eps'
