
function X=rx_bachelor(Nsamples)
%
% function X=rx_bachelor(Nsamples)
% 
%
% Simple receive function for use in the Bachelors project course 2014.
%
%           X: Received complex samples.
%    Nsamples: Number of samples to receive


RF_freq=1784.9e6; % Center frequency.
ref_clk=0;
pps_trigger=0;
gain=200;
rx_rate=1e5;
LOoffset=10e6;

X=rx(Nsamples+20000,RF_freq,ref_clk,gain,rx_rate,LOoffset);
X=X(20001:(20000+Nsamples));
