n = 7;
title_string = {'wiki' ...
    'skitter' ...
    'livejournal' ...
    'hollywood' ...
    'orkut' ...        
    'sinaweibo' ...
    'friendster'};

for i = 1:n
    figure(i);
    title(title_string(i));
    plot(comp(i,1:5)); hold on;
    plot(notie(i,1:5)); hold on;
    plot(tiefull(i,1:5)); 
    xlabel_hand=xlabel('Number of landmarks');
    set(xlabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);
    ylabel_hand=ylabel('Avg. errorr ratio');
    set(ylabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);
    legend_hand = legend('TreeSketch','Dec. Search','Tie breaker DS');
    set(legend_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);
end

% set(gca,'Xlim',[0 10],'XTick',1:1:9,'XTickLabel',{'Wiki' 'Skitter' 'Baidu' 'Livejournal' 'hollywood' 'orkut' 'sinaweibo' 'webuk' 'friendster'});
% set(gca, 'Fontname', 'Times New Roman', 'Fontsize', 15);
% ylabel_hand=ylabel('Avg. errorr ratio');
% set(ylabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);
% legend_hand = legend('TreeSketch','Dec. Search','Tie breaker DS','Orientation','horizontal');
% set(legend_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);