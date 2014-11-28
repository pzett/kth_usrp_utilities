function  [P,F]=plotting_phase_noise(y,clr)

BW=0.1e6;
Ts=1e-9;
Ns=floor(length(y)/2)-1;
t=(-Ns:Ns)*Ts+1e-13;

FFT_y=abs(fft(y));
[dummy,max_ix]=max(abs(FFT_y));
f0=(max_ix-1)/length(y)*1e9;
if (f0>0.5e9)
    f0=f0-1e9;
end;

h=sin(pi*BW*t)./t;
h=h/max(abs(h));
Nf=1000;
P=zeros(Nf,1);
F=zeros(Nf,1);

for i1=1:Nf
    f=f0+(i1-1)*BW;
    temp=exp(-j*2*pi*f*t').*y(1:length(t));
    P(i1)=abs(h*temp)^2;
    F(i1)=f-f0;
end;

if exist('clr')
    semilogx(F(2:end),10*log10(P(2:end)/((F(2)-F(1))*P(1))),clr)
end;
