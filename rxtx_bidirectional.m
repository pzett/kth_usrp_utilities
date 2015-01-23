function X=rxtx_bidirectional(Nsamples,Y,freq_tx,freq_rx,rate,num_repeats,send_to_listener)
%
% X=rxtx_bidirectional(Nsamples,Y,freq_tx,freq_rx,rate)
%

filename_rx='data_from_usrp.dat';
filename_tx='data_to_usrp.dat';
freq=0;


if (max(abs([real(Y),imag(Y)])>2^15-1))
   error('Signal is out of range, the legal range is -2^15 to 2^15-1'); 
end;

temp=zeros(1,2*Nsamples);
for i1=1:size(Y,2)
   temp(i1*2-1)=real(Y(1,i1));
   temp(i1*2-0)=imag(Y(1,i1));
end;

fid=fopen(filename_tx,'w');
fwrite(fid,temp,'int16');
fclose(fid);


cmd_str=['sudo ./rxtx_bidirectional '];
cmd_str=[cmd_str,' --nsamp ',num2str(Nsamples)];
cmd_str=[cmd_str,' --freq_tx ',num2str(freq_tx),...
         ' --freq_rx ',num2str(freq_rx),...
' --rate ',num2str(rate),' '];
cmd_str=[cmd_str,' --nrep=',num2str(num_repeats)];
cmd_str=[cmd_str,' --n=',num2str(send_to_listener)];

system(cmd_str);
cmd_str

fid=fopen(filename_rx,'r');
temp=fread(fid,inf,'int16');
X=[temp(1:2:end)'+j*temp(2:2:end)'];
fclose(fid);

system(['rm -f ',filename_rx]);
system(['rm -f ',filename_tx]);




