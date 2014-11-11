X=rx_60GHz(60e9,5000,0,10,50e6,-2,'192.168.10.2');
Xt=X;

[ start_pos, f_offset] = synchronize_OFDM1(X(1:2000), parameters,1,1, 1);
X=X.*exp(-j*2*pi*f_offset*(1:length(X)));
X=X((start_pos+140):(start_pos+2100));


ix1=1:100;
ix2=ix1+200;
f_offset2=angle((mean(X(ix2))/mean(X(ix1))))/(2*pi*(ix2(1)-ix1(1)));
X=X.*exp(-j*2*pi*f_offset2*(1:length(X)));

ix1=1:100;
ix2=ix1+400;
f_offset3=angle((mean(X(ix2))/mean(X(ix1))))/(2*pi*(ix2(1)-ix1(1)));
X=X.*exp(-j*2*pi*f_offset3*(1:length(X)));