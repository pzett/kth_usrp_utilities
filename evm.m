function [percentage, error] = evm( tx,rx )

    tx=tx(:);
    rx=rx(:);
    error=tx-rx;
    Perror=mean(abs(error).^2);
    Preference=mean(abs(tx).^2);
    percentage=sqrt(Perror/Preference)*100;
    dbs=10*log10(Perror/Preference);


end

