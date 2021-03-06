
% Fre-space path-loss
% 
% FSPL = 20log(d) + 20log(f) - 147.55
% Max gain in circuit 38dB
% Gain in antenna 7.5dBi
% Output power 16dBm
% EIRP: 23.5dBm
% Loss 0.2m: 54dB

usrp_addr='192.168.10.2';
gain_tx=13;
rf_freq=60e9;
rate=25e6;
low_res=0;

%X=5000*exp(j*2*pi*0*(0:9999))*sqrt(0.5);
X=5000*exp(j*2*pi*0.1*(0:9999))+5000*exp(-j*2*pi*0.1*(0:9999));
X=X*2;

tx_60GHz(rf_freq, length(X), X,0, gain_tx, rate, low_res, usrp_addr);

%% Experiment 21/10-2014
%% Two CW
%% gain_tx=13, Base-band ampl=10000+10000. Rec. 
%% saved 14:43

%% Experiment 21/10-2014
%% Two CW
%% gain_tx=13, Base-band ampl=5000+5000. Rec. 
%% Measurement:
%% M1 -26.27dBm
%% D2 -5.035MHz, -2.02dB
%% D3 -9.99MHz, -17.5dB
%% D4 4.955MHz, -20.9dB
%% D5 9.99MHz, -25.6dB
%% D6 -15.025MHz, -26.4dB


%% Experiment 21/10-2014
%% Two CW
%% gain_tx=13, Base-band ampl=2500+2500. Rec. 
%% Measurement:
%% M1 -30.15dBm
%% D2 -5.035MHz, -1.46dB
%% D3 -9.99MHz, -21.5dB
%% D4 4.955MHz, -22.4dB
%% D5 9.99MHz, -35dB
%% D6 -15.025MHz, -36.3dB

%% Experiment 21/10-2014
%% Two CW
%% gain_tx=13, Base-band ampl=(2500+2500)/sqrt(2). Rec. 
%% Measurement:
%% M1 -32.37dBm
%% D2 -5.035MHz, -1.22dB
%% D3 -9.99MHz, -24.6dB
%% D4 4.955MHz, -24.3dB
%% D5 9.99MHz, -45dB
%% D6 -15.025MHz, -45dB

%% Experiment 21/10-2014
%% Two CW
%% gain_tx=13, Base-band ampl=(2500+2500)/2. Rec. 
%% Measurement:
%% M1 -34.7dBm
%% D2 -5.035MHz, -1.07dB
%% D3 -9.99MHz, -29.2dB
%% D4 4.955MHz, -28.6dB
%% D5 9.99MHz, -52dB. Below noise.
%% D6 -15.025MHz, -52dB. Below noise.
%% Saved 14:16


%% Experiment 21/10-2014
%% Two CW
%% gain_tx=10, Base-band ampl=5000+5000. Rec. 
%% Measurement:
%% M1 -28.2dBm. 59.931836GHz
%% D2 -5.035MHz, -1.72dB
%% D3 -9.99MHz, -19.2dB
%% D4 4.955MHz, -21.2dB
%% D5 9.99MHz, -29.5dB
%% D6 -15.025MHz, -31.5dB
%% saved 14:21



%% Experiment 21/10-2014
%% Two CW
%% gain_tx=10, Base-band ampl=2500+2500. Rec. 
%% Measurement:
%% M1 -32.4dBm. 59.931836GHz
%% D2 -5.035MHz, -1.2dB
%% D3 -9.99MHz, -24.4dB
%% D4 4.955MHz, -24.14dB
%% D5 9.99MHz, -47dBdB
%% D6 -15.025MHz, -46dB

%% Experiment 21/10-2014
%% Two CW
%% gain_tx=10, Base-band ampl=(2500+2500)/2. Rec. 
%% Measurement:
%% M1 -37.3dBm. 59.931836GHz
%% D2 -5.035MHz, -1.0dB
%% D3 -9.99MHz, -35.2dB
%% D4 4.955MHz, -34.6dB
%% D5 9.99MHz, -49dB. Below noise
%% D6 -15.025MHz, -50dB. Below noise
%% saved 14:27

%% Experiment 21/10-2014
%% Two CW
%% gain_tx=7, Base-band ampl=10000+10000. Rec. 
%% Measurement:
%% M1 -26.8dBm. 59.931836GHz
%% D2 -5.035MHz, -2.0dB
%% D3 -9.99MHz, -16.5dB
%% D4 4.955MHz, -18.5dB
%% D5 9.99MHz, -24.6dB
%% D6 -15.025MHz, -27.6dB
%% saved 1437

%% Experiment 21/10-2014
%% Two CW
%% gain_tx=7, Base-band ampl=5000+5000. Rec. 
%% Measurement:
%% M1 -29.8dBm. 59.931836GHz
%% D2 -5.035MHz, -1.5dB
%% D3 -9.99MHz, -20.7dB
%% D4 4.955MHz, -21.6dB
%% D5 9.99MHz, -33.6dB
%% D6 -15.025MHz, -34.7dB


%% Experiment 21/10-2014
%% Two CW
%% gain_tx=7, Base-band ampl=2500+2500. Rec. 
%% Measurement:
%% M1 -34.3dBm. 59.931836GHz
%% D2 -5.035MHz, -1.1dB
%% D3 -9.99MHz, -28.1dB
%% D4 4.955MHz, -27.4dB
%% D5 9.99MHz, -52.6dB
%% D6 -15.025MHz, -52.8dB


%% Experiment 21/10-2014
%% Single CW
%% gain_tx=13, Base-band ampl=5000. Rec. level main component: -25.5dBm
%% DC component: 25.8dB down, intermod at -70MHz. 31.8dB down 
%% Distance 24cm. intermod at +140MHz 32.2dB down.


%% Single CW
%% gain_tx=13, Base-band ampl=5000*sqrt(0.5). 
%% Rec. level main component: -27.5dBm
%% DC component: 18.5dB down, intermod at -70MHz. 30.5dB down 
%% Distance 24cm. +140MHz 31.7dB down.

%% Single CW
%% gain_tx=13, Base-band ampl=2500
%% Rec. level main component: -29.7dBm
%% DC component: 16dB down, intermod at -70MHz. 31.0dB down 
%% Distance 24cm. +140MHz 31.1dB down.


%% Single CW
%% gain_tx=10, Base-band ampl=5000. Rec. level main component: -33.3dBm
%% DC component: 25.8dB down, intermod at -70MHz. 33.24dB down 
%% Distance 24cm.

%% Single CW
%% gain_tx=10, Base-band ampl=5000*sqrt(0.5). 
%% Rec. level main component: -29.9dBm
%% DC component: 17.7dB down, intermod at -70MHz. 34.1dB down 
%% Distance 24cm. +140MHz -30.3dB down.

%% Single CW
%% gain_tx=10, Base-band ampl=2500;
%% Rec. level main component: -32.44dBm
%% DC component: 15.6dB down, intermod at -70MHz. 37.0dB down 
%% Distance 24cm. +140MHz -29.7dB down.


%% Single CW
%% gain_tx=7, Base-band ampl=5000;
%% Rec. level main component: -29.3dBm
%% DC component: 16.9dB down, intermod at -70MHz. 35.8dB down 
%% Distance 24cm. +140MHz -30.0dB down.


%% Single CW
%% gain_tx=7, Base-band ampl=5000*sqrt(0.5);
%% Rec. level main component: -31.7dBm
%% DC component: 19.2dB down, intermod at -70MHz. 38.6dB down 
%% Distance 24cm. +140MHz -29.45dB down.


%% Single CW
%% gain_tx=7, Base-band ampl=2500;
%% Rec. level main component: -34.6dBm
%% DC component: 13.7dB down, intermod at -70MHz. 42.2dB down 
%% Distance 24cm. +140MHz 28.8dB down.


%% Single CW
%% gain_tx=4, Base-band ampl=5000;
%% Rec. level main component: -31.5dBm
%% DC component: 19.4dB down, intermod at -70MHz. 40.5dB down 
%% Distance 24cm. +140MHz -29.4dB down.


%% Single CW
%% gain_tx=4, Base-band ampl=5000*sqrt(0.5);
%% Rec. level main component: -34.2dBm
%% DC component: 18.9dB down, intermod at -70MHz. 44.4dB down 
%% Distance 24cm. +140MHz -29.0dB down.


%% Single CW
%% gain_tx=4, Base-band ampl=2500;
%% Rec. level main component: -37.0dBm
%% DC component: 14.3dB down, intermod at -70MHz. 47.1dB down 
%% Distance 24cm. +140MHz -28.8dB down.


%% Single CW
%% gain_tx=0, Base-band ampl=5000;
%% Rec. level main component: -37.6dBm
%% DC component: 20.1dB down, intermod at -70MHz. 37.6dB down 
%% Distance 24cm. +140MHz -29.4dB down.

%% Single CW
%% gain_tx=0, Base-band ampl=5000*sqrt(0.5);
%% Rec. level main component: -40.5dBm
%% DC component: 15.5dB down, intermod at -70MHz. 45.6dB down 
%% Distance 24cm. +140MHz -29.2dB down.

%% Single CW
%% gain_tx=0, Base-band ampl=2500;
%% Rec. level main component: -46.7dBm
%% DC component: 11.2dB down, intermod at -70MHz. 39.8dB down (below noise)
%% Distance 24cm. +140MHz -29.8dB down.

