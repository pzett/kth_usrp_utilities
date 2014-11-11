
function [f,SNR]=estSNR(X)
%
Nfft=2^20;
temp_fft=fft(X,Nfft);
[dummy,k]=max(abs(abs(temp_fft)));
A=temp_fft(k)/min(length(X),Nfft);
Noise=X-A*exp(j*2*pi*(k-1)*(0:(length(X)-1))/Nfft);
f=(k-1)/Nfft*25e6;
if (f>12.5e6)
    f=f-25e6;
end;
SNR=abs(A)^2;
SNR=SNR/mean(abs(Noise).^2);

%% std noise 6.4915
