
function X=rx_60GHz(RF_freq,Nsamples,ref_clk,gain,rx_rate,scaling_8bits,dev_addr,clock_freq,BasicDB_freq)
%
%  function X=rx(RF_freq, Nsamples,ref_clk,pps_trigger,gain,rx_rate,scaling_8bits,dev_addr,clock_freq,BasicDB_freq)
%
%       RF_freq: Center frequency of 60GHz RX board. 
%             X: Received complex samples.
%      Nsamples: Number of samples to receive
%       RF_freq: Center frequency of 60GHz RX board.
%       ref_clk: If =1 then the receiver is locked to the external 10MHz REFCLOCK. 
%               Set it to zero if you don't know.
%          gain: Receiver gain of analog section.
%       rx_rate: Sample-rate. Default 25MHz.
% scaling_8_bits: If ==0 then 16bits is used. If <>0 then 8 bits are used. 
%	          If >0 then scaling_bits should then be set to the maximum 
%                 amplitude expected divided by 2^15.
%                 If <0 then scaling_8_bits is automatically set
%                 using a safety margin of abs(scaling_8_bits).
%                 The parameter scaling_bits should then be set to the maximum 
%                 amplitude expected.
%	dev_addr: IP address of USRP.
%     clock_freq: Clock frequency of CLK board (in MHz). Default 285.714MHz.
%   BasicDB_freq: Set the carrier frequency used between the USRP and the
%                 daughterboard.



filename='d.dat';
LOoffset=0; 

if ~exist('rx_rate')
    rx_rate=25e6;
end;
if ~exist('LOoffset')
    LOoffset=0;
end;
if ~exist('scaling_8bits')
    scaling_8bits=0;
end;
if ~exist('dev_addr')
  dev_addr='192.168.10.2';
end;
if ~exist('clock_freq')
  clock_freq=285.714;
end;
if isempty('clock_freq')
  clock_freq=285.714;
end;
if isempty(clock_freq)
  clock_freq=285.714;
end;
if ~exist('BasicDB_freq')
  BasicDB_freq=70e6;
end;


cmd_str=['sudo ./rx_60GHz --nsamp=',num2str(Nsamples),' --filename=',filename];
cmd_str=[cmd_str,' --freq=',num2str(BasicDB_freq),' --rxrate=',num2str(rx_rate)];
cmd_str=[cmd_str,' --rf_freq=',num2str(RF_freq)];
cmd_str=[cmd_str,' --gain=',num2str(gain)];
cmd_str=[cmd_str,' --LOoffset=',num2str(LOoffset)];
cmd_str=[cmd_str,' --dev_addr=',dev_addr];
cmd_str=[cmd_str,' --clock_freq=',num2str(clock_freq)];



if (abs(scaling_8bits)>0)
  cmd_str=[cmd_str,' --8bits_scaling=',num2str(scaling_8bits)];
end;


system(cmd_str);

%pause(10)
fid=fopen(filename,'r');
temp=fread(fid,inf,'int16');
X=[temp(1:2:end)'-j*temp(2:2:end)'];
fclose(fid);

% Remove the file which the samples were written on
system(['rm -f ',filename]);
