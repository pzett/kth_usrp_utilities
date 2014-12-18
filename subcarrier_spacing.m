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

% This script generates the experimental sub-carrier spacing
% plot of of the paper
% Open Source SDR Frontend and Measurements for 60GHz Wireless Experimentation
%

% Point to location of measurement data.
meas_dir='/home/perz/meas141017/'; 

if 0

Nffts=[100,200,400,800,1600,3200]; %% Number of subcarriers of 1GHz
M=16;
Ss=3200; %segment spacing
Nsegments=10;
Ms=[4,16,64,256];
SNR=100;

BER=zeros(length(Nffts),length(Ms));
for i1=1:200 % 200 measurement files.
    i1

    [X,Y]=get_data2([meas_dir,'meas',num2str(i1)]);
    X=X(1:99900)';
    Y=Y(1:99900)';
    y = filter_phase_noise(X+j*Y);
  
    for s_i=1:Nsegments        
        for n_fft_i=length(Nffts):-1:1

           Nfft=Nffts(n_fft_i);
           ix=(s_i-1)*Ss+(1:Nfft);
                  
           yn=y(ix);
           yn=yn/sum(yn); % Remove CPE
           yn=yn/mean(abs(yn)); % Nornalize power      
 
           for m_i=1:length(Ms)
          
               
               M=Ms(m_i);
               p=qammod(0:(M-1),M);
               t=p*p'/length(p);
               p=(1/Nfft)*t;
                             
               
        	data_in=floor(rand(Nfft,1)*M);
                q = qammod(data_in,M);
                q=q+inv(SNR*sqrt(p)*2)*(randn(size(q))+j*randn(size(q)));
                q=q.*yn;
                s=ifft(q);
                
                
                data_hat = qamdemod(fft(s),M);
                BER(n_fft_i,m_i)=BER(n_fft_i,m_i)+biterr(data_in,data_hat)/(Nsegments*200*log2(M)*length(data_in));
            end;
         end;
     end;
  end; 
  save BERres BER Nffts
  
end;

load BERres
hold off
Fig=gcf;
set(Fig,'Color',[1 1 1]);
grid
plot(Nffts,BER(:,1),'-x');
hold on
plot(Nffts,BER(:,2),'-s');
plot(Nffts,BER(:,3),'-d');
plot(Nffts,BER(:,4),'-p');


xlabel('Nfft');
ylabel('BER');
legend('QPSK','16QAM','64QAM','256QAM',2);
set(gca,'FontSize',16);
x=get(gca,'Xlabel');
set(x,'FontSize',16);
y=get(gca,'Ylabel');
set(y,'FontSize',16);
grid

'print -deps subcarrier.eps'
