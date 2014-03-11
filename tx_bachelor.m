function tx_bachelor(X)
%
% function tx_bachelor(X)
%
% Simple transmit function for use in the Bachelors project course 2014.
% General description:
% Loops the signal X until interrupted by the user.
% 

Nsamples=length(X);
RF_freq=1784.9e6;
ref_clk=0;
gain=20;
tx_rate=1e5;
LOoffset=2e6;

tx( Nsamples,RF_freq, X, ref_clk , gain, tx_rate, LOoffset)
