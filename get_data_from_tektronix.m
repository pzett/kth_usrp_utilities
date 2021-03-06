
function  get_data_from_tektronix( filename )
%
% function  get_data_from_tektronix( filename )
% 
% This function reads data from a Tektronix DPO2014
% oscilloscope using the visa object in Matlab.
% The function is hardcoded to use two channels,
% as is the address of the instrument. The competent
% user should be able to changes these hard coded
% properties.
% The data is saved in two text files with 
% with filenames derived from the filename input string.
% The filename input string should be given without extension.
% The data is read from the textfiles using the companion
% function get_data2.

buff_size=64;

addr='USB::0x0699::0x0373::C012929::INSTR';
vs=visa('tek',addr);
fopen(vs);
fwrite(vs,'DATa:ENCdg ASCii');

fwrite(vs,'ACQuire:STOPAfter   SEQuence');
fwrite(vs,'ACQuire:STATE RUN');
pause(10);
fwrite(vs,'ACQuire:STATE STOP');



for ch=1:2
    if ch==1
        fwrite(vs,'DATa:SOUrce CH1');
    else
        fwrite(vs,'DATa:SOUrce CH2');    
    end;

    fwrite(vs,'wavfrm?');
    fid=fopen([filename,'_ch=',num2str(ch),'.txt'],'wb');

    c=fread(vs,buff_size,'char');
    co=buff_size;
    while ~isempty(c) && co==buff_size
        fwrite(fid,c,'char');
        [c,co]=fread(vs,buff_size,'char');
    
    end;
    fclose(fid);
end;

fwrite(vs,'ACQuire:STOPAfter   RUNSTop');
fwrite(vs,'ACQuire:STATE RUN');
fclose(vs);
fclose(vs);
