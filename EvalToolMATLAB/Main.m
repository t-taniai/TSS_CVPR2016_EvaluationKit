
DatasetRootDir = '..\Dataset\';
ResultsRootDir = '..\Results\';

%% Evaluation of results.
% The output file "scores.csv" is saved as e.g. "..\Results\FG3DCar\scores.csv".
RunEvaluation([ResultsRootDir, 'FG3DCar'], [DatasetRootDir, 'FG3DCar']);
% RunEvaluation([ResultsRootDir, 'PASCAL'], [DatasetRootDir, 'PASCAL']);
% RunEvaluation([ResultsRootDir, 'JODS'], [DatasetRootDir, 'JODS']);

%% Visualization of results. 
% The output images are saved as e.g. "..\Results\FG3DCar\001_005\vis\*.png".
RunVisualization([ResultsRootDir, 'FG3DCar'], [DatasetRootDir, 'FG3DCar'], 'vis');
% RunVisualization([ResultsRootDir, 'PASCAL'], [DatasetRootDir, 'PASCAL'], 'vis');
% RunVisualization([ResultsRootDir, 'JODS'], [DatasetRootDir, 'JODS'], 'vis');