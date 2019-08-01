function plotChannel(eeg,chan)
fig = figure('Position',[100 100 1480 740])
colormap('jet');
imagesc(flipud(squeeze(eeg(chan,:,:))'));

end
