k_all=x;
h=bar(k_all);
set(gca,'Xlim',[0 10],'XTick',1:1:9,'XTickLabel',{'Wiki' 'Skitter' 'Baidu' 'Livejournal' 'hollywood' 'orkut' 'sinaweibo' 'webuk' 'friendster'});
set(gca, 'Fontname', 'Times New Roman', 'Fontsize', 15);
ylabel_hand=ylabel('Avg. errorr ratio');
set(ylabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);
legend_hand = legend('TreeSketch','Dec. Search','Tie breaker DS','Orientation','horizontal');
set(legend_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);