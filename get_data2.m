
function [x,y] = get_data2 (filename )
%
% function [x,y] = get_data2 (filename )
%
% Read data from two channels save on text-files
% using get_data_from_tektronix.
%
% filename: Filename including full path but excluding the
%           "_ch=1.txt" and "_ch=2.txt" filename endings.
%
% x,y     : Signal received from channel one and two of
%           the oscilloscope corresponding to I and Q.
    
for ch=2:-1:1
    
  fid=fopen([filename,'_ch=',num2str(ch),'.txt'],'rb'); 
    
  s='';
  while ~strcmp('100000000',s)
    c=char(fread(fid,1,'char'));
    d=';';
    s='';
    while ~(c==d)
        s=[s,c];
        c=char(fread(fid,1,'char'));
    end;
  end;

  str=fread(fid,inf,'char');
  fclose(fid);
  
  if (str(end)==45)
    str=str(1:(end-2));
  end;
    
  if ch==1
    x=str2num((char(str')));
  else
    y=str2num((char(str')));   
  end;
  
 
end;

end

