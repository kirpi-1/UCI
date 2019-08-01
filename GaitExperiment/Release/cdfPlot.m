function cdfPlot(header,data)
    names = fieldnames(data);
    
    for i=1:size(names)       
        h = figure();
        set(h,'Name',char(names(i)));
        for j=1:header.numFreq
            subplot(9,1,j);
            plot(data.(char(names(i)))(j,:));        
        end
    end
end