@echo off

set evaltool="%~dp0x64\Release\EvalTool.exe"
set datasetroot=%~dp0..\Dataset

if [%1]==[] (
set resultsroot=%~dp0..\Results
) else (
set resultsroot=%~1
)

%evaltool% -resultsDir "%resultsroot%\FG3DCar" -datasetDir "%datasetroot%\FG3DCar" -mode visualization -bgColor 000255255 -flbgColor 128128128 -visSubDir vis
%evaltool% -resultsDir "%resultsroot%\JODS" -datasetDir "%datasetroot%\JODS" -mode visualization -bgColor 000255255 -flbgColor 128128128 -visSubDir vis
%evaltool% -resultsDir "%resultsroot%\PASCAL" -datasetDir "%datasetroot%\PASCAL" -mode visualization  -bgColor 000255255 -flbgColor 128128128 -visSubDir vis

echo Done.
pause;
