n = 8;
c = 4;
r = 2;
title_string = {'Wiki' ...
    'Skitter' ...
    'Livejournal' ...
    'Hollywood' ...
    'Orkut' ...        
    'Sinaweibo' ...
    'Webuk'...
    'Friendster'};
legend_string = {'TreeSketch'... 
    'DS Single Branch'...
    'DS Single Branch with heuristic index'...
    'DS Full Branch'...
    'DS Full Branch with heuristic index'};

figure(1);
for i = 1:n
    subplot(r, c, i);
    plot(comp(i,1:8), '--^', 'LineWidth', 2, 'Markers', 7); hold on;
    plot(notie(i,1:8), '-d', 'LineWidth', 2, 'Markers', 7); hold on;
    plot(notielabel(i,1:8), '-x', 'LineWidth', 2, 'Markers', 7); hold on;
    plot(tiefull(i,1:8), '-o', 'LineWidth', 2, 'Markers', 7); hold on;
    plot(tiefulllabel(i,1:8), '-+', 'LineWidth', 2, 'Markers', 7);
    
    set(gca,'FontSize',15);
    
%     y_limit = max(comp(i,2) * 1.5, comp(i, 1));
%     ylim([0 y_limit]);
%     if y_limit > 0.1
%         set(gca,'YTick',(0:0.1:y_limit));
%     else
%         set(gca,'YTick',(0:0.01:y_limit));
%     end
    xlim([1 8]);
    set(gca,'XTick',(1:8));
    set(gca,'XTickLabel',{'1'; '2'; '3'; '4'; '5'; '10'; '15'; '20'});
    
    xlabel_hand=xlabel('Number of landmarks');
    set(xlabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 20);
    ylabel_hand=ylabel('Average error ratio');
    set(ylabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 20);
%     legend_hand = legend(legend_string);
%     set(legend_hand,'Fontname', 'Times New Roman', 'Fontsize', 10);
    title(title_string(i));
end
legend(legend_string,...
    'Orientation', 'horizontal',...
    'Position', [0.02 0.004 0.96 0.029]);