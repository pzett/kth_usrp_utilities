load train.mat


%Insignal
training=r1;
payload = ones(1,880);
guard = zeros(1,16);
x= [training payload guard];
x= kron(x, [1 0 0 0])


%Filter
osf = 4;
Fd = 0.195312E6; %transfer sampling frequency
Fs = 4*Fd; %filter sampling frequency
beta = 0.75; %rolloff factor
[gTXn,gTXd] = rcosine(Fd,Fs,'sqrt',beta);


%Filtrerad insignal

x= filter(gTXn,gTXd,x);
x= 5000*x;

%Define the number of samples as inparameter to rx_bachelor
Nsample=length(x)



%Call tx_bachelor to continously transfer the signal
tx_bachelor(x)