load train.mat
%Final v.

%r=r1;%
r=[-1 1 -1 -1 1 -1 1 1 1 -1 1 -1 -1 -1 1]

%Nsample=1024*4*2 %Double Buffering.Assuming frame length of 1024. Oversampling factor 4.
Nsample=(15+100+16)*4*2
y=rx_bachelor(Nsample);
lpayload=100*4;
%y=abs(y);
storA=0;
storB=0;
storF=0;
corrLength2=2*Nsample-1;
frequency=0;
frekspann=-30000:100:30000;
Ts=1/100000; %tx-rate 100k symbols per second according to the file tx_bachelor
c = r;
nyv= [];
nr=0;
%skapar training sequence

%Padding reference training sequence to be able to find a match.
for nri=1:15
    nyv(nri+nr) = c(nri);
    nyv(nri+nr+1) = 0;  
    nyv(nri+nr+2) = 0;
    nyv(nri+nr+3) = 0;
    nr = nr + 3;
end
langdnyvVISA=length(nyv)

%kollar korskorrelationen mellan y och trainingsekvensen
for frek=frekspann
    ykomp=y.*exp(-1j*2*pi*frek*Ts*(1:length(y)));
    korrY=fftshift(ifft(fft(ykomp,corrLength2).*conj(fft(nyv,corrLength2))));
    [a,b] = max(abs(korrY));
    if a > storA %om a �r st�rre �n det tidigare maximum
       storA = a; %d� s�tter vi storA=a. Detta g�rs tills storA �r maximal. vi hittar d� den maximala korskorrelationen
       storB = b;
       storF = frek;
       %plot(abs(korrY));
       %title('cross correlation y and training sequence')
       %str=sprintf('delta f=%d , traning sequence ends in sample=%d', storF, storB);
       %xlabel(str)
       
    end
    
end

storA %1.3465e+003
storB %9979 den maximala korrelationen sker vid lag 9979
storF


svarB=storB-length(y)
svarS=storB-length(y)+length(nyv)


nyVektorY=y(svarS:svarS+lpayload);
plot(nyVektorY)


%*******************************************************************************
%R�knar ut channel estimation h0hatt. Anv�nder formel (26)i kompendiet main.pdf.
k=1;
h0sum=0;
h1sum=0;

nyvsconjugate=conj(nyv);
while k<length(nyvsconjugate)+1
    h0=exp(-1j*2*pi*storF*Ts)*y(k)*nyvsconjugate(k);
    h0sum=h0sum+h0;
    h1=(abs(nyv(k)))^2;
    h1sum=h1sum+h1;
    k=k+1;
end
h0hatt=h0sum/h1sum; %channel estimation

%R�knar ut noise variance med formel (25) i kompendiet.
v1sum=0;
l=1;
    while l<length(nyv)+1
        v1=(abs(y(l)*exp(-1j*2*pi*storF*Ts)-h0hatt*nyv(l)))^2;
        v1sum=v1sum+v1;
        l=l+1;
    end
variance=v1sum/length(nyv)

%R�knar ut SNR med hj�lp av variansen och h0hatt. Utan n�gon
%frequency-offset blir det samma formel som innan.Anv�nder formel (20).
%absolutbeloppet av v�ntev�rdet p� v�r �versamplade training sequence

B=(abs(nyv)).^2
M=mean(B); %
h=(abs(h0hatt))^2;
SNR=h/variance %SNR=1,58
SNRdb=10*log(SNR) %SNR in decibel
%***************************************************************************



ykomp=y.*exp(-1j*2*pi*storF*Ts*(1:length(y)));
korrY=fftshift(ifft(fft(ykomp,corrLength2).*conj(fft(nyv,corrLength2))));

subplot(4,1,1)
plot(nyv)
title('Traning sequence')
xlabel('Samples')
ylabel('Amplitude')
subplot(4,1,2)
plot(abs(y))
title('Received signal')
xlabel('Samples')
ylabel('Amplitude')
subplot(4,1,3)
plot(abs(korrY))
title('Cross correlation')
xlabel('Samples*2')
ylabel('Cross correlation magnitude')
subplot(4,1,4)
plot(abs(nyVektorY))
title('Payload')
xlabel('Samples')
ylabel('Amplitude')
%str=sprintf('delta f=%d     stor b=%d     SNR(dB)=%d', storF, storB, SNRdb);
%xlabel(str)
