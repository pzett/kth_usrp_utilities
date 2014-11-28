
% This script generates the phase-noise plot of the paper
% Open Source SDR Frontend and Measurements for 60GHz Wireless Experimentation
%

% Point to location of measurement data.
meas_dir='/home/perz/meas141017/'; 


[Xn,Yn]=get_data2([meas_dir,'noise1']);
Xn=Xn(1:99900)';
Yn=Yn(1:99900)';

if 1
for i1=1:-1
   i1
   [X,Y]=get_data2([meas_dir,'meas',num2str(i1)]);
   X=X(1:99900)';
   Y=Y(1:99900)';
   y = filter_phase_noise(X+j*Y,Xn+j*Yn);
   [P,F]=plotting_phase_noise(y);
   if i1==1
     P0=P/200;
   else
     P0=P0+P/200;
   end;
end;
%save phase_noise_plot_data_filtered

for i1=1:200
   i1
   [X,Y]=get_data2([meas_dir,'meas',num2str(i1)]);
   X=X(1:99900)';
   Y=Y(1:99900)';
   
   px=mean(abs(X).^2);
   py=mean(abs(Y).^2);

   y=X+j*Y*sqrt(px/py);

   [P,F]=plotting_phase_noise(y);
   if i1==1
     P0=P/200;
   else
     P0=P0+P/200;
   end;
end;

save phase_noise_plot_data

end;

hold off
load phase_noise_plot_data
P1=10*log10(P0(2:end)/((F(2)-F(1))*P0(1)));
semilogx(F(2:end),P1,'k');

hold on
load phase_noise_plot_data_filtered
P1=10*log10(P0(2:end)/((F(2)-F(1))*P0(1)));
semilogx(F(2:end),P1,'k--');


xlimits=[100e3,1e6,10e6,50e6];
ylimits=zeros(size(xlimits));

for i1=1:length(xlimits)
  [dummy,ix]=min(abs(F-xlimits(i1)));
  ylimits(i1)=P1(ix);
end;

str='';
for i1=1:length(xlimits)
   str=[num2str(xlimits(i1)/1e6),' MHz: ',num2str(round(ylimits(i1)*10)/10), 'dbc/Hz '];
   text(1.5e5,-95-i1*8,str,'FontSize',16);
end;


Fig=gcf;
set(Fig,'Color',[1 1 1]);
grid
xlabel('Frequency Hz');
ylabel('Power spectral density dB/Hz');
set(gca,'FontSize',16);

x=get(gca,'Xlabel');
set(x,'FontSize',16);
y=get(gca,'Ylabel');
set(y,'FontSize',16);

'print plot1.eps -depsc'
axis([1e5 1e8 -130 -70]);

