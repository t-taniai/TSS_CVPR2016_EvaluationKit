@echo off

set evaltool=%~dp0x64\Release\EvalTool.exe
set datasetroot=%~dp0..\Dataset
set resultsroot=%~dp0..\Results

%evaltool% -resultsDir %resultsroot%\FG3DCar -datasetDir %datasetroot%\FG3DCar -mode evaluation
rem %evaltool% -resultsDir %resultsroot%\JODS -datasetDir %datasetroot%\JODS -mode evaluation
rem %evaltool% -resultsDir %resultsroot%\PASCAL -datasetDir %datasetroot%\PASCAL -mode evaluation

echo Done.
