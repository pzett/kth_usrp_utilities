
function [hard_bits,hd,rx,power] = demod_OFDM4( waveform, parameters, start_pos, MMSE)
%
% Demodulator corresponding to the mod_OFDM4 modulator.
%
% Inputs:
% waveform:     The complex base-band input signal (number_of_antennasx number_of_samples)
% parameters:   output from the modulator function modem_OFDM4.
% start_pos:    Position in waveform from which the samples of the first OFDM
%               symbol is taken.
% MMSE:         If =1, linear MMSE receiver, =0 Maximum ratio combining.
%
% Outputs
% hard_bits:    Detected bits (zeros and ones)
% h        :    Channel estimate of the desired user.
% power    :    RMS power of the waveform samples used for detection.

% Give paramterers shorter names
Nfft=parameters.Nfft;
Np=parameters.Np;
Ns=parameters.Nsymbols;
ix_all=parameters.ix_all;
known_symbol=parameters.known_symbol;
known_pos=parameters.known_pos;
interf_pos=parameters.interf_pos;
scaling_of_known=parameters.scaling_of_known;
ix=parameters.ix;
Const=parameters.constellation;
b2s=parameters.b2s; %% Bit to symbol mapping
use_pilot_subcarriers=parameters.use_pilot_subcarriers;
prepend_sync_seq=parameters.prepend_sync_seq;
re_order=parameters.re_order;
pilot_symbol=parameters.pilot_symbol;
no_bits_per_symb=round(log2(length(Const)));

if ~exist('MMSE')
    MMSE=1;
end;


% Estimate interfering channel
Hi=zeros(2,length(ix_all),length(interf_pos));
hi=zeros(2,Nfft);

if length(interf_pos)>0
for ant=1:size(waveform,1)
  for i1=1:length(interf_pos)
    est_pos=start_pos+(interf_pos(i1)-1)*(Nfft+Np);
    received_symbol=fft(waveform(ant,est_pos+(0:(Nfft-1))));
    power=mean(abs(waveform(ant,est_pos+(0:(Nfft-1)))).^2);
    temp=received_symbol;
    ht=temp./(conj(known_symbol(:,i1)')*scaling_of_known+1e-12);
    ht=ht(ix_all);
    Hi(ant,:,i1)=ht;
  end;

  Hit=squeeze(Hi(ant,:,:));
  ha=mean(abs(Hit(:,:)),2);
  ph0=exp(-j*angle(mean(Hit(:,:)./repmat(Hit(:,1),1,length(interf_pos)))));
  Hit=Hit.*repmat(ph0,length(ix_all),1);
  hp=angle(mean(Hit,2));
  h=ha.*exp(j*hp);
  Hi(ant,:,:)=Hit;
  hi(ant,ix_all)=h;
end;
end;

% Estimate desired channel
Hd=zeros(2,length(ix_all),length(known_pos));
hd=zeros(2,Nfft);

power=0;
for ant=1:size(waveform,1)
  for i1=1:length(known_pos)
    est_pos=start_pos+(re_order(known_pos(i1))-1)*(Nfft+Np);
    received_symbol=fft(waveform(ant,est_pos+(0:(Nfft-1))));
    if (ant==1) && (i1==1)
      rs=received_symbol;
    end;
    power=power+mean(abs(waveform(ant,est_pos+(0:(Nfft-1)))).^2);
    temp=received_symbol;
    ht=temp./(conj(known_symbol(:,i1)')*scaling_of_known+1e-12);
    ht=ht(ix_all);
    Hd(ant,:,i1)=ht;
    
  end;

  Hdt=squeeze(Hd(ant,:,:));

  ha=mean(abs(Hdt(:,:)),2);
  ph0=exp(-j*angle(mean(Hdt(:,:)./repmat(Hdt(:,1),1,length(known_pos)))));
  Hdt=Hdt.*repmat(ph0,length(ix_all),1);
  hp=angle(mean(Hdt,2));
  h=ha.*exp(j*hp);
  hd(ant,ix_all)=h;

  
end;


if ~parameters.use_pilot_subcarriers
    ix=ix_all;
end;

hard_bits=zeros(   (Ns-length(known_pos))*length(ix)*no_bits_per_symb,1);
ConstBig=repmat(Const(b2s),length(ix),1);
rx=zeros(length(ix),Ns-length(known_pos));


if (size(waveform,1)==1) %% Single antenna
h_all=hd(1,:);
i10=0;
for i1=1:Ns
   if isempty(find(i1==known_pos))

     pos=start_pos+(re_order(i1)-1)*(Nfft+Np);
     received_symbol=fft(waveform(pos+(0:(Nfft-1))));
     power=power+mean(abs( waveform(pos+(0:(Nfft-1)))).^2);

     demodulator_input=received_symbol./h_all;
     is_value=demodulator_input(use_pilot_subcarriers(1));
     cpec=pilot_symbol./is_value;
     CPECS(i10+1)=cpec;
     cpec=cpec/abs(cpec);
     demodulator_input=cpec*demodulator_input(ix);   
     demodulator_input=conj(demodulator_input');
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
return;
end;



%% Two antennas
i10=0;
i20=0;
for i1=1:Ns
   if isempty(find(i1==known_pos))

     pos=start_pos+(re_order(i1)-1)*(Nfft+Np);
     received_symbol1=fft(waveform(1,pos+(0:(Nfft-1))));
     received_symbol2=fft(waveform(2,pos+(0:(Nfft-1))));
     power=power+mean(abs( waveform(1,pos+(0:(Nfft-1)))).^2);
     power=power+mean(abs( waveform(2,pos+(0:(Nfft-1)))).^2);

     % Desired pilot subcarrier. Antenna 1.
     temp=received_symbol1(use_pilot_subcarriers(1));
     temp=temp/hd(1,use_pilot_subcarriers(1));
     cpec=temp/pilot_symbol;
     cpec=cpec/abs(cpec);
     hdt(1,:)=hd(1,:)*cpec;
     cpecd1=cpec;


     % Desired pilot subcarrier. Antenna 2.
     temp=received_symbol2(use_pilot_subcarriers(1));
     temp=temp/hd(2,use_pilot_subcarriers(1));
     cpec=temp/pilot_symbol;
     cpec=cpec/abs(cpec);
     hdt(2,:)=hd(2,:)*cpec;
     cpecd2=cpec;

     % Interf pilot subcarrier. Antenna 1.
     temp=received_symbol1(use_pilot_subcarriers(2));
     temp=temp/hi(1,use_pilot_subcarriers(2));
     cpec=temp/pilot_symbol;
     cpec=cpec/abs(cpec);
     hit(1,:)=hi(1,:)*cpec;
     
     % Interf pilot subcarrier. Antenna 2.
     temp=received_symbol2(use_pilot_subcarriers(2));
     temp=temp/hi(2,use_pilot_subcarriers(2));
     cpec=temp/pilot_symbol;
     cpec=cpec/abs(cpec);
     hit(2,:)=hi(2,:)*cpec;
     i30=0;

     
     for i2=ix

       H=zeros(2,2);
       H(:,1)=hdt(:,i2);         
       H(:,2)=hit(:,i2);  
       s=[received_symbol1(i2);received_symbol2(i2)];


       if MMSE
	 R=H*H';
	 w=inv(H(:,2)*H(:,2)'+diag(diag(R))*0.1)*H(:,1);
       else
	 w=H(:,1);
       end;

       shat=w'*s;
       shat=shat/(w'*H(:,1));
     

       % Desired signal channel
       [dummy,selected]=min(abs(shat(1)-Const));
       temp=dec2bin(selected-1,no_bits_per_symb)=='1';
       temp=temp';
       b1=no_bits_per_symb*i20+1;
       b2=b1+no_bits_per_symb-1;
       
       hard_bits(b1:b2)=temp(:);
       rx(i30+1,i10+1)=shat;
       i20=i20+1;
       i30=i30+1;

    end;
    i10=i10+1;
  end;  

end;

power=power/Ns;

end

