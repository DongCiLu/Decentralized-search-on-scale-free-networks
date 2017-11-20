n = 8;
label_string = {'Wiki' ...
    'Skitter' ...
    'Livejournal' ...
    'Hollywood' ...
    'Orkut' ...        
    'Sinaweibo' ...
    'Webuk'...
    'Friendster'};

legend_string = {'DS single branch' ...
    'DS full branch' ...
    'DS parallel single branch' ...
    'DS parallel full branch'};   
    
figure(1);
h = bar(search_time);


ybuff=2;
for i=1:length(h)
    XDATA=bsxfun(@plus, h(1).XData, [h.XOffset]');
    YDATA=cat(1, h.YData);
    for j=1:size(XDATA, 2)
        x=XDATA(i,j);
        y=YDATA(i,j);
        if y > 1000
            scale=1000;
            unit='k';
        else
            scale=1;
            unit='';
        end
        t=[num2str(y / scale, 3) ,unit];
        text(x,y,t,'Color','k','HorizontalAlignment','left','Rotation',90,'FontSize',20)
    end
end

set(gca, 'YScale', 'log');
set(gca,'FontSize',20);
set(gca, 'XTickLabel', label_string);
ylim([0 10e5]);

ylabel_hand=ylabel('Throughput (queries / second)');
set(ylabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 20);
legend_hand = legend(legend_string);
set(legend_hand,'Fontname', 'Times New Roman', 'Fontsize', 20, 'Location', 'best', 'Orientation', 'Horizontal');
