function [  start_pos, f_offset] = synchronize_OFDM1 ( waveform, parameters , estimate_freq_offset, T, oversamp)

Nfft_sync=2^16;
train=parameters.train;
Nlags=length(waveform)-oversamp*length(train)+1;
Criterion=zeros(1,Nlags);
FrequencyIx=zeros(1,Nlags);

if (size(waveform,2)<size(waveform,1))
    waveform=conj(waveform');
end;

m=size(waveform,1);
train=kron(train,[1,zeros(1,oversamp-1)]);
train_big=repmat(conj(train),m,1);

if (estimate_freq_offset)

    if (m==1)
        for i1=1:Nlags
            de_spread=waveform(i1-1+(1:length(train))).*conj(train);
            [p,fi]=max(abs(fft(de_spread,Nfft_sync)));
            Criterion(i1)=p;
            FrequencyIx(i1)=fi;
        end;
    else
        for i1=1:Nlags
            de_spread=waveform(:,i1-1+(1:length(train))).*train_big;
            [p,fi]=max(sum(abs(fft(de_spread',Nfft_sync)').^2));
            Criterion(i1)=p;
            FrequencyIx(i1)=fi;
        end;
    end;
        

    [dummy  start_pos]=max(abs(Criterion));
    f_offset=(FrequencyIx(start_pos)-1)/Nfft_sync;
    if (f_offset>0.5)
        f_offset=f_offset-1;
    end;
    f_offset=f_offset*inv(T)*oversamp;
    
else
    
    if (m==1)
        for i1=1:Nlags
            de_spread=waveform(i1-1+(1:length(train))).*conj(train);
            Criterion(i1)=abs(sum(de_spread));
        end
    else
        for i1=1:Nlags
            de_spread=waveform(:,i1-1+(1:length(train))).*train_big;
            Criterion(i1)=sum(abs(sum(de_spread',2)).^2);
        end;
    end;
    [dummy  start_pos]=max(abs(Criterion));
    f_offset=0;

    
end;
