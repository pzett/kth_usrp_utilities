function [hard_bits,h,rx,power,CPECS] = demod_OFDM4( waveform, parameters, start_pos)
%
% Demodulator corresponding to the mod_OFDM1 modulator.
%
% Inputs:
% waveform:     The complex base-band input signal.
% parameters:   output from the modulator function.
% start_pos:    Position in waveform from which the samples of the first OFDM
%               symbol is taken.
%
% Outputs
% hard_bits:    Detected bits (zeros and ones)
% h        :    Channel estimate.
% power    :    RMS power of the waveform samples used for detection.

% Give paramterers shorter names
Nfft=parameters.Nfft;
Np=parameters.Np;
Ns=parameters.Nsymbols;
ix_all=parameters.ix_all;
known_symbol=parameters.known_symbol;
known_pos=parameters.known_pos;
scaling_of_known=parameters.scaling_of_known;
ix=parameters.ix;
Const=parameters.constellation;
b2s=parameters.b2s; %% Bit to symbol mapping
use_pilot_subcarriers=parameters.use_pilot_subcarriers;
prepend_sync_seq=parameters.prepend_sync_seq;
re_order=parameters.re_order;
pilot_symbol=parameters.pilot_symbol;



if (size(waveform,1)==1)
    waveform=conj(waveform');
end;
no_bits_per_symb=round(log2(length(Const)));

% Estimate the channel in the frequency domain
H=zeros(length(ix_all),length(known_pos));

for i1=1:length(known_pos)
  est_pos=start_pos+(re_order(known_pos(i1))-1)*(Nfft+Np);
  received_symbol=fft(waveform(est_pos+(0:(Nfft-1))));
  power=mean(abs(waveform(est_pos+(0:(Nfft-1)))).^2);
  temp=conj(received_symbol');
  h_all=temp./(conj(known_symbol(:,i1)')*scaling_of_known+1e-12);
  h=h_all(ix_all);
  H(:,i1)=h;
end;


ha=mean(abs(H),2);
ph0=exp(-j*angle(mean(H./repmat(H(:,1),1,length(known_pos)))));
H=H.*repmat(ph0,length(ix_all),1);
hp=angle(mean(H,2));
h=ha.*exp(j*hp);
h_all=zeros(size(h_all));
h_all(ix_all)=h;

if ~parameters.use_pilot_subcarriers
    ix=ix_all;
end;
hard_bits=zeros(   (Ns-length(known_pos))*length(ix)*no_bits_per_symb,1);
ConstBig=repmat(Const(b2s),length(ix),1);
rx=zeros(length(ix),Ns-length(known_pos));


i10=0;
for i1=1:Ns
   if isempty(find(i1==known_pos))

     pos=start_pos+(re_order(i1)-1)*(Nfft+Np);
     received_symbol=fft(waveform(pos+(0:(Nfft-1))));
     power=power+mean(abs( waveform(pos+(0:(Nfft-1)))).^2);

     demodulator_input=received_symbol./conj(h_all');
     is_value=demodulator_input(use_pilot_subcarriers(1));
     cpec=pilot_symbol./is_value;
     CPECS(i10+1)=cpec;
     cpec=cpec/abs(cpec);
     demodulator_input=cpec*demodulator_input(ix);   
     rx(:,i10+1)=demodulator_input;
     demodulator_input=repmat(demodulator_input,1,length(Const));
     [dummy,selected]=min(abs(demodulator_input-ConstBig),[],2);
     temp=dec2bin(selected-1,no_bits_per_symb)=='1';
     temp=temp';
     b1=length(ix)*no_bits_per_symb*i10+1;
     b2=b1+length(ix)*no_bits_per_symb-1;
     hard_bits(b1:b2)=temp(:);
     i10=i10+1;
   
  end;  

end;

power=power/Ns;

end

