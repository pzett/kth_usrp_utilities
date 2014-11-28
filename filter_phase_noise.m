
function y = filter_phase_noise(x,xn)
%
% y = filter_phase_noise(x,xn)
%
% x:  Complex valued input signal.
% xn: Noise only measurement (optional).
%     If supplied the SNR is displayed.
%
%  y: Downconverted and filtered output signal. 
%     The filter bandwidth is set to 
%     0.97 times the frequency of the fundamental.
%     The filter thereby removing overtones.

x=x-mean(x);

if exist('xn')

  xn=xn-mean(xn);
  
end;
   



px=mean(abs(real(x)).^2);
py=mean(abs(imag(x)).^2);

x=real(x)+j*imag(x)*sqrt(px/py);


FFT_x=abs(fft(x));
[dummy,max_ix]=max(abs(FFT_x));
f0  =(max_ix-1)/length(x)*1e9

if (f0>0.5e9)
    f0=f0-1e9;
end;



%% LP Filtering
Ts=1e-9;
Ns   =floor(length(x)/4)-2;
t   =(-Ns:Ns)*Ts+1e-13;
f1=f0*0.97;
h    =exp(2i*pi*f0*t).*sinc(2*f1*t)*(2*f1*Ts);
y =conv(x,h);
y=y(length(h):(end-length(h)-1));
y=y.*exp(-j*2*pi*f0*((1:length(y))')*Ts);



if exist('xn')
  yn=conv(xn,h);
  yn=yn(length(h):(end-length(h)-1));
  yn=yn.*exp(-j*2*pi*f0*((1:length(y))')*Ts);
  disp(['SNR=',num2str(10*log10(abs(y'*y)/abs(yn'*yn))),' dB']);
  
end;

