
ix_start=min(find(abs(X)>100));

ix=(ix_start+100):(ix_start+900);
Nfft=2^14;
Y=20*log10(abs(fft(X(ix),Nfft)));

[dummy,max_ix]=max(abs(Y));
if (max_ix>Nfft)
  max_ix=max_ix-Nfft;
end;

f=(max_ix-1)/Nfft;