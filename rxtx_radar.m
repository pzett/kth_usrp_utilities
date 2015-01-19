
function X=rxrx_radar(Nsamples,Y,RF_freq,rate)
%
% X=rxrx_radar(Nsamples,Y,RF_freq,rate)
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


cmd_str=['sudo ./rxtx_radar '];
cmd_str=[cmd_str,' --nsamp ',num2str(Nsamples)];
cmd_str=[cmd_str,' --freq ',num2str(RF_freq),' --rate ',num2str(rate),' '];



system(cmd_str);

fid=fopen(filename_rx,'r');
temp=fread(fid,inf,'int16');
X=[temp(1:2:end)'+j*temp(2:2:end)'];
fclose(fid);

system(['rm -f ',filename_rx]);
system(['rm -f ',filename_tx]);




