k_all=x;
h=bar(k_all);
set(gca,'Xlim',[0 10],'XTick',1:1:9,'XTickLabel',{'Google' 'Wiki' 'Skitter' 'Gene' 'Baidu' 'Facebook' 'Livejournal' 'hollywood' 'friendster'});
set(gca, 'Fontname', 'Times New Roman', 'Fontsize', 15);
ylabel_hand=ylabel('Avg. errorr ratio');
set(ylabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);
legend_hand = legend('LCA distance','Dec. Search','Bidirectional DS','Tie breaker DS','All opt on','10x landmarks','Orientation','horizontal');
set(legend_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);