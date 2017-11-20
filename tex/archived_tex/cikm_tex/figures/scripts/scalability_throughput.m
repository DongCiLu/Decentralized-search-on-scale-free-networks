legend_string = {'Wiki' ...
    'Skitter' ...
    'Livejournal' ...
    'Hollywood' ...
    'Orkut' ...        
    'Sinaweibo' ...
    'Webuk'...
    'Friendster'};

markers = ['-+'; '-o'; '-*'; '-^'; '-x'; '-d'; '-s'; '-p'];

n = 5;
figure(1);
for i = 1:n
    plot(scal_m(i,:), markers(i,:), 'LineWidth', 2, 'Markers', 7); hold on;
end

set(gca,'FontSize',20);
set(gca,'XTick',(1:5));
set(gca,'XTickLabel',{'1'; '2'; '4'; '8'; '16'});
xlabel_hand=xlabel('Number of machines');
set(xlabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 20);
ylabel_hand=ylabel('Throughput (queries / second)');
set(ylabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 20);
legend_hand = legend(legend_string(1:n));
set(legend_hand,'Fontname', 'Times New Roman', 'Fontsize', 20, 'Location', 'NorthWest');

n=8;
figure(2);
for i = 1:n
    plot(scal_q(i,:), markers(i,:), 'LineWidth', 2, 'Markers', 7); hold on;
end
set(gca,'FontSize',20);
set(gca,'XTick',(1:5));
set(gca,'XTickLabel',{'100K'; '200K'; '400K'; '800K'; '1.6M'});
xlabel_hand=xlabel('Number of queries');
set(xlabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 20);
ylabel_hand=ylabel('Throughput (queries / second)');
set(ylabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 20);
legend_hand = legend(legend_string(1:n));
set(legend_hand,'Fontname', 'Times New Roman', 'Fontsize', 20, 'Location', 'NorthWest');
