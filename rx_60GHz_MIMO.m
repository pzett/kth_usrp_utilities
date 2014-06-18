
function X=rx_60GHz_MIMO(Nsamples,gain,rx_rate,LOoffset,scaling_8bits)
%
%  function X=rx_60GHz_MIMO(Nsamples,ref_clk,pps_trigger,gain,rx_rate,LOoffset,scaling_8bits,dev_addr)
% 
%             X: Received complex samples.
%      Nsamples: Number of samples to receive
%          gain: Receiver gain of analog section.
%       rx_rate: Sample-rate. Default 25MHz.
%       LOoffset: Offset between RF LO and actually used frequency.
% scaling_8_bits: If ==0 then 16bits is used. If <>0 then 8 bits are used. 
%	          If >0 then scaling_bits should then be set to the maximum 
%                 amplitude expected divided by 2^15.
%                 If <0 then scaling_8_bits is automatically set
%                 using a safety margin of abs(scaling_8_bits).



filename='d.dat';
RF_freq=70e6;

if ~exist('rx_rate')
    rx_rate=25e6;
end;
if ~exist('LOoffset')
    LOoffset=0;
end;
if ~exist('scaling_8bits')
    scaling_8bits=0;
end;


cmd_str=['sudo ./rx_60GHz_MIMO --nsamp=',num2str(Nsamples),' --filename=',filename];
cmd_str=[cmd_str,' --freq=',num2str(RF_freq),' --rxrate=',num2str(rx_rate)];
cmd_str=[cmd_str,' --gain=',num2str(gain)];
cmd_str=[cmd_str,' --LOoffset=',num2str(LOoffset)];

%if (pps_trigger)
%    cmd_str=[cmd_str,' --PPS=true '];
%end;
if (abs(scaling_8bits)>0)
  cmd_str=[cmd_str,' --8bits_scaling=',num2str(scaling_8bits)];
end;

system(cmd_str);

no_chans=2;
fid=fopen(filename,'r');
temp=fread(fid,inf,'int16');
fclose(fid);
for i1=1:no_chans
    ix_start=2*Nsamples*(i1-1)+1;
    ix_start1=ix_start+1;
    ix_stop=2*Nsamples*(i1-1)+2*Nsamples-1;
    ix_stop1=ix_stop+1;
    X(i1,:)=temp(ix_start:2:ix_stop)-j*temp(ix_start1:2:ix_stop1);
end;
system(['rm -f ',filename]);
