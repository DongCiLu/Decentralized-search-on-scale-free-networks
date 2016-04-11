% low = y(:);
% high = z(:);
% combined = [low, high];
% 
% [n,m]=size(combined);
% C=reshape(combined',m,2,[]);
% A=permute(C,[2 1 3]);
% p=size(A,3);
% B=[A;zeros(1,m,p)];
% out=[0,0];
% for k=1:9
%     out=[out;B(:,:,k)];
% end

x(:,:,1)=y;
x(:,:,2)=z;

k_all=x;
groupLabels={'Google' 'Wiki' 'Skitter' 'Gene' 'Baidu' 'Facebook' 'Livejournal' 'hollywood' 'friendster'};
plotBarStackGroups(k_all, groupLabels)
set(gca, 'Fontname', 'Times New Roman', 'Fontsize', 15);
ylabel_hand=ylabel('Avg. errorr ratio');
set(ylabel_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);
legend_hand = legend('Greedy index LCA distance','Random index LCA distance','Greedy index Dec. search','Random index Dec. search');
set(legend_hand,'Fontname', 'Times New Roman', 'Fontsize', 15);