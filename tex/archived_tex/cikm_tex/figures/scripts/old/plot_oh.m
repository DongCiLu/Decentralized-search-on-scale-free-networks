k_all=x;
h=plot(k_all);
%set(gca,'Xlim',[0 10],'XTick',1:1:9,'XTickLabel',{'Google' 'Wiki' 'Skitter' 'Gene' 'Baidu' 'Facebook' 'Livejournal' 'hollywood' 'friendster'});
set(gca, 'YScale', 'log');
set(gca, 'Fontname', 'Times New Roman', 'Fontsize', 11);
ylabel_hand=ylabel('Searching overhead [\mus]');
set(ylabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 13);
legend_hand = legend('DS wo/ early termination','DS w/ early termination', 'Bidirectional DS', 'Tie break DS');
set(legend_hand,'Fontname', 'Times New Roman', 'Fontsize', 13);