@echo off

set evaltool="%~dp0x64\Release\EvalTool.exe"
set datasetroot=%~dp0..\Dataset

if [%1]==[] (
set resultsroot=%~dp0..\Results
) else (
set resultsroot=%~1
)

%evaltool% -resultsDir "%resultsroot%\FG3DCar" -datasetDir "%datasetroot%\FG3DCar" -mode evaluation -autoFlip 1
%evaltool% -resultsDir "%resultsroot%\JODS" -datasetDir "%datasetroot%\JODS" -mode evaluation -autoFlip 1
%evaltool% -resultsDir "%resultsroot%\PASCAL" -datasetDir "%datasetroot%\PASCAL" -mode evaluation -autoFlip 1

echo Done.
pause;
