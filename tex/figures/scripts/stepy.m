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

figure(1);
for i = 1:n
    subplot(r, c, i);
    plot(comp(i,1:8), '-^', 'LineWidth', 2, 'Markers', 5); hold on;
    plot(notie(i,1:8), '-d', 'LineWidth', 2, 'Markers', 5); hold on;
    plot(notie(i,1:8), '-x', 'LineWidth', 2, 'Markers', 5); hold on;
    plot(tiefull(i,1:8), '-o', 'LineWidth', 2, 'Markers', 5); hold on;
    plot(tiefull(i,1:8), '-+', 'LineWidth', 2, 'Markers', 5);
    
    set(gca,'FontSize',12);
    
    y_limit = max(comp(i,2) * 1.5, comp(i, 1));
    ylim([0 y_limit]);
    if y_limit > 0.1
        set(gca,'YTick',(0:0.1:y_limit));
    else
        set(gca,'YTick',(0:0.01:y_limit));
    end
    xlim([1 8]);
    set(gca,'XTick',(1:8));
    set(gca,'XTickLabel',{'1'; '2'; '3'; '4'; '5'; '10'; '15'; '20'});
    
    xlabel_hand=xlabel('Number of landmarks');
    set(xlabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);
    ylabel_hand=ylabel('Average error ratio');
    set(ylabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);
    legend_hand = legend('TreeSketch', 'DS Single Branch', 'DS Single Branch w/ heur index', 'DS Full Branch', 'DS Full Branch w/ heur index');
    set(legend_hand,'Fontname', 'Times New Roman', 'Fontsize', 10);
    title(title_string(i));
end