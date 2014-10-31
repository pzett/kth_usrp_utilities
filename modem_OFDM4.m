function [waveform, parameters]=modem_OFDM4(Nsymbols,modulation_ix,known_pos,use_pilot_subcarriers,prepend_sync_seq,bits_in,re_order)
%
%
% function [waveform, parameters]=modem_OFDM4(Nsymbols,known_pos,use_pilot_subcarriers,prepend_sync_seq,bits_in,re_order)
%
% This is a OFDM modulation intended for use with 25MHz or 50MHz sample-frequency (Fs),
% the carrier spacing is either 625kHz or 1250kHz, respectively.
%
% The function creates a burst of Nsymbols OFDM symbols. The OFDM is based
% on an FFT of length Nfft=40. The time domain signal is defined according
% to the IFFT but with a cyclic prefix i.e.
%
%                           Nfft-1
% waveform(n+Np) = (1/Nfft) sum  C(k)*exp( j*2*pi*k*(n-1)/Nfft), 0 <= n <= Nfft
%                           k=0
%
% waveform(n)=waveform(n+Nfft), 1<=n<=Np,
%
% where Np is length of the cyclic prefix. Subcarrier indecies 1...9 and
% 33..40 are modulated while the remaining are nulled. Thanks to the
% folding at Fs, this correspond to carrier frequencies 0 to
% 5MHz and -5MHz to -0.625MHz at 25MHz sample-rate.
% With 50MHz sample-rate the corresponding number are 0 to 10MHz and
% -
%
% One of the OFDM symbols is entirely known at the receiver. This is symbol
% as index known_pos if the symbold are numbered 1...Ns. This symbols uses
% only QPSK as subcarrier modulation.
%
% In order to facilitate phase-derotation, for SISO and MIMO systems with independent
% phase-noise between transmitter branches, the use_pilot_subcarriers parameter has been 
% introduced in a rather involved way.
% 
% For SISO systems, use_pilot_subcarriers points at the subcarrier [1,..9,33..40], which contains 
% a single pilot symbol 1+j, which is used for phase-derotation in the receiver.
% For MIMO systems, use_pilot_subcarriers is a vector. The first element use_pilot_subcarriers(1) is used
% to point out a pilot symbol 1+j, which is used for phase-derotation as in the SISO case.
% The remaining elements use_pilot_subcarriers(2:end) are used to point out subcarriers
% wich are not modulated but filled with zeros symbols 0+j0 (i.e. no power transmitted).
% If use_pilot_subcarriers contains numbers which are outside [1,..9,33..40], 
% these elements are simply ignored.     
% 
%
% If prepend_sync_seq=1 then a synchronization sequence is prepended to the
% waveform. This sequence facilitates synchronization in the time and
% frequency domain.
%
% The struct parameters should provide all information needed for
% demodulation.
%
% The vector bits_in are binary input which is transmitted. If bits_in is
% to short it will be padded with random data.
%
% The variable modulation_ix determines the constellation
% =4 (QPSK), =16 (16QAM), =64 (64QAM)
%
% The parameter re_order, if defined, can be used to re-order the position of the 
% the OFDM symbols. If re_order(x)=y then the x:th symbol is transmitted on
% the nominal location of the y:th symbol. Note that y can be larger than Nsymbols. 
% This is useful for implementing e.g. antenna hopping. 

if ~exist('re_order')
   re_order=1:Nsymbols;
end;
if isempty(re_order)
    re_order=1:Nsymbols;
end;
if ~exist('interleave')
    interleave=0;
end;
if isempty(interleave)
    interleave=0;
end;


% constants
Nfft=40;
Np=4;
gap=3;
train=[1,1,-1,1,1,1,1,-1,1,1,1,-1,1,1,1,-1,-1,-1,1,1,1,1,...
        -1,-1,-1,-1,-1,1,-1,-1,1,-1,1,1,-1,1,1,1,-1,1,1,1,1,-1,-1,-1,...
        -1,1,1,1,1,1,1,-1,-1,-1,1,1,-1,1,-1,-1,-1,-1,-1,-1,1,-1,-1,1,...
        1,1,1,-1,1,-1,-1,-1,-1,1,1,-1,1,-1,-1,-1,1,1,-1,1,-1,-1,-1,...
        -1,1,1,-1,-1,1,-1,1,-1,1,1,1,1,1,-1,1,1,1,-1,1,-1,-1,1,-1,-1,...
        1,1,-1,1,1,1,1,1,1,-1,1,-1,-1,-1,1,-1,-1,1,-1,1]; % Length 138

%Phase shifting the training sequence by 90 deg. will be no problem?
%train = train * j;


ix_all= [1:9,33:40];

ix=[];
ix_sub=[];
i4=1;
use_pilot_subcarriers_sub=[];
for i1=1:length(ix_all)
   i3=ix_all(i1);
   is_pilot_subcarrier=0;
   for i2=1:length(use_pilot_subcarriers)
     if (use_pilot_subcarriers(i2)==i3)
       is_pilot_subcarrier=1;
       use_pilot_subcarriers_sub=[use_pilot_subcarriers_sub,i4];
     end;
   end;
   if (~is_pilot_subcarrier)
     ix=[ix,i3];
     ix_sub=[ix_sub,i4];
   end;
   i4=i4+1;
end;

Constellation4=[0.707107+0.707107i -0.707107+0.707107i 0.707107-0.707107i -0.707107-0.707107i];
b2s4=[0 1 2 3]+1;

Constellation16=[0.948683+0.948683i 0.316228+0.948683i -0.316228+0.948683i -0.948683+0.948683i 0.948683+0.316228i 0.316228+0.316228i -0.316228+0.316228i -0.948683+0.316228i 0.948683-0.316228i 0.316228-0.316228i -0.316228-0.316228i -0.948683-0.316228i 0.948683-0.948683i 0.316228-0.948683i -0.316228-0.948683i -0.948683-0.948683i];
b2s16=[0 1 3 2 4 5 7 6 12 13 15 14 8 9 11 10]+1; %% Symbol mapping

Constellation64=[1.08012+1.08012i 0.771517+1.08012i 0.46291+1.08012i 0.154303+1.08012i -0.154303+1.08012i -0.46291+1.08012i -0.771517+1.08012i -1.08012+1.08012i 1.08012+0.771517i 0.771517+0.771517i 0.46291+0.771517i 0.154303+0.771517i -0.154303+0.771517i -0.46291+0.771517i -0.771517+0.771517i -1.08012+0.771517i 1.08012+0.46291i 0.771517+0.46291i 0.46291+0.46291i 0.154303+0.46291i -0.154303+0.46291i -0.46291+0.46291i -0.771517+0.46291i -1.08012+0.46291i 1.08012+0.154303i 0.771517+0.154303i 0.46291+0.154303i 0.154303+0.154303i -0.154303+0.154303i -0.46291+0.154303i -0.771517+0.154303i -1.08012+0.154303i 1.08012-0.154303i 0.771517-0.154303i 0.46291-0.154303i 0.154303-0.154303i -0.154303-0.154303i -0.46291-0.154303i -0.771517-0.154303i -1.08012-0.154303i 1.08012-0.46291i 0.771517-0.46291i 0.46291-0.46291i 0.154303-0.46291i -0.154303-0.46291i -0.46291-0.46291i -0.771517-0.46291i -1.08012-0.46291i 1.08012-0.771517i 0.771517-0.771517i 0.46291-0.771517i 0.154303-0.771517i -0.154303-0.771517i -0.46291-0.771517i -0.771517-0.771517i -1.08012-0.771517i 1.08012-1.08012i 0.771517-1.08012i 0.46291-1.08012i 0.154303-1.08012i -0.154303-1.08012i -0.46291-1.08012i -0.771517-1.08012i -1.08012-1.08012i];
b2s64=[0 1 3 2 7 6 4 5 8 9 11 10 15 14 12 13 24 25 27 26 31 30 28 29 16 17 19 18 ...
    23 22 20 21 56 57 59 58 63 62 60 61 48 49 51 50 55 54 52 53 32 33 35 34 39 38 36 37 40 41 43 42 47 46 44 45]+1;

pilot_symbol=1+j;
known_symnol=zeros(Nfft,2);
known_symbol(1:9,1)=[  1 + 1i ,1 - 1i,-1 + 1i,-1 + 1i,-1 + 1i ,1 + 1i ,-1 + 1i ,-1 + 1i,1+i];
known_symbol(33:40,1)=[1 - 1i   1 - 1i  -1 - 1i   1 - 1i  -1 + 1i   1 - 1i  -1 - 1i  -1 + 1i];
known_symbol(1:9,2)=[1 - 1i -1 + 1i -1 - 1i,1 + 1i   1 + 1i  -1 + 1i   1 - 1i   1 - 1i   1 - 1i];
known_symbol(33:40,2)=[-1 + 1i   1 + 1i  -1 + 1i  -1 + 1i  -1 + 1i  -1 + 1i  -1 - 1i  -1 - 1i];
scaling_of_known=1/sqrt(2);

switch (modulation_ix)
    case 4
        Const=Constellation4;
        b2s=b2s4;
    case 16
        Const=Constellation16;
        b2s=b2s16;
    case 64
        Const=Constellation64;
        b2s=b2s64;
    otherwise
        error('This constallation is not defined');
end;



parameters.Nfft=Nfft;
parameters.Nsymbols=Nsymbols;
parameters.Np=Np;
parameters.re_order=re_order;
parameters.train=train;
parameters.gap=gap; %% Gap between training sequence and data.
parameters.ix_all=ix_all; %% Subcarriers used for pilots and data.
parameters.ix=ix; %% Subcarriers used for data
parameters.use_pilot_subcarriers=use_pilot_subcarriers;
parameters.use_pilot_subcarriers_sub=use_pilot_subcarriers_sub;
parameters.constellation=Const;
parameters.scaling_of_known=scaling_of_known;
parameters.known_symbol=known_symbol;
parameters.known_pos=known_pos;
parameters.prepend_sync_seq=prepend_sync_seq;

parameters.b2s=b2s; %% Bit to symbol mapping
parameters.pilot_symbol=pilot_symbol; 

if ~exist('bits_in')
    bits_in=[];
end;
bits_in=bits_in(:);

Ns=Nsymbols;
no_bits_per_symb=round(log2(length(Const)));

number_of_bits_needed=(Ns-length(known_pos))*length(ix)*no_bits_per_symb;
tx=zeros(length(ix),Ns-length(known_pos));
bits_in=[bits_in;(rand(number_of_bits_needed-length(bits_in),1)>0.5)];

parameters.number_of_bits_needed=number_of_bits_needed;


waveform=zeros(1,max(re_order)*(Nfft+Np));
burst=zeros(length(ix_all),1);
i10=0;
power=0;
i3=1;

for i1=1:Ns
    if ~isempty(find(i1==known_pos))
       burst=scaling_of_known*known_symbol(:,i3);
       i3=i3+1;
       if (i3>size(known_symbol,2))
	 i3=1;
       end;
       burst(use_pilot_subcarriers_sub(2:end));
    else
       for i2=1:length(ix)
          ixx=(i2-1)*no_bits_per_symb+(1:no_bits_per_symb)+i10*length(ix)*no_bits_per_symb;
          burst(ix(i2))=Const(b2s(bin2dec(num2str(bits_in(ixx))')+1));
       end;
       burst(use_pilot_subcarriers_sub(1))=scaling_of_known*pilot_symbol;
       tx(:,i10+1)=burst(ix); 
       i10=i10+1;
    end;
    
    temp=ifft(burst,Nfft);
    ixx=Np+(1:Nfft)+(Np+Nfft)*(re_order(i1)-1);
    waveform(ixx)=temp;
    power=power+mean(abs(temp).^2);
    ixx=(1:Np)+(Np+Nfft)*(re_order(i1)-1);
    waveform(ixx)=temp(end+(-(Np-1):0));
   
    parameters.tx=tx;    

end;

parameters.power=power/(Ns-1);

if (prepend_sync_seq==1)
    waveform=[0.7*train*max(abs(waveform))/max(abs(train)),zeros(1,gap),waveform];
end;
if (prepend_sync_seq==2)
    waveform=[zeros(size(train)),zeros(1,gap),waveform];
end;


end
 
 
