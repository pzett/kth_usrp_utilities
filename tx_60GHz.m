function tx_60GHz(RF_freq, Nsamples, X, ref_clk , gain, tx_rate, low_res, dev_addr)



%
% 
% function tx_60GHz( RF_freq, Nsamples, X, ref_clk, gain, tx_rate, low_res, dev_addr)
%
% General description:
% Zero-pads the signal X to length Nsamples and then loops
% the signal until interrupted by the user.
% 
%
%  RF_freq: Center frequency of 60GHz RX board.
% Nsamples: Repeat pattern length.
%        X: Transmitted signal. Can be double but is intepreted as short.
%  ref_clk: If =1 then the transmitter is locked to REF CLOCK.
%           Set it to zero if you don't know.
%     gain: Gain of the analog transmitter chain.
%  tx_rate: Sample-rate. Default 25MHz.
%  low_res: If =1 then 8bits per sample (real and imag) is used.
%           The code automatically sets the "peak" value for
%           the UHD driver.
% dev_addr: IP address of USRP.

filename='data_to_usrp.dat';
LOoffset=0;
BasicDB_freq=70e6;

if ~exist('tx_rate')
    tx_rate=25e6;
end;
if ~exist('low_res')
    low_res=0;
end;
if ~exist('dev_addr')
    dev_addr='192.168.10.2';
end;


if (size(X,2)<size(X,1))
    X=conj(X');
end;

if (max(abs([real(X),imag(X)])>2^15-1))
   error('Signal is out of range, the legal range is -2^15 to 2^15-1'); 
end;

if (max(abs(X))<10)
   fprintf(1,'I believe your input signal has to small amplitude! \n');
   fprintf(1,'Press any key to continue! \n');
   pause;
end;

temp=zeros(1,2*Nsamples);
for i1=1:size(X,2)
   temp(i1*2-1)=real(X(1,i1));
   temp(i1*2-0)=-imag(X(1,i1));
end;
 

fid=fopen(filename,'w');
fwrite(fid,temp,'int16');
fclose(fid);


cmd_str=['sudo ./tx_60GHz '];

cmd_str=[cmd_str,' --nsamp ',num2str(Nsamples),' --filename ',filename];
cmd_str=[cmd_str,' --freq=',num2str(BasicDB_freq),' --txrate=',num2str(tx_rate)];
cmd_str=[cmd_str,' --rf_freq=',num2str(RF_freq)];
cmd_str=[cmd_str,' --gain ',num2str(gain),' '];
cmd_str=[cmd_str,' --forever true '];
cmd_str=[cmd_str,' --LOoffset=',num2str(LOoffset)];
cmd_str=[cmd_str,' --dev_addr=',dev_addr];



if (ref_clk)
    cmd_str=[cmd_str,' --10MHz true '];
end;
if (low_res)
    cmd_str=[cmd_str,' --8bits true '];
end;



system(cmd_str);
