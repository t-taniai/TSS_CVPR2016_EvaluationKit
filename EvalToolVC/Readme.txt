---------
How to use:
---------

RunEvaluation.bat is the main script for evaluation.
RunVisualization.bat is the main script for visualization.
By double clicking the bat files, evaluation/visualization will be done on sample results in /TSS_CVPR2016_EvaluationKit/Results.
By drag-and-dropping the /Results folder (or any folder that has the same structure), you can also evaluate/visualize data in that folder.

RunEvaluationPrec.bat demonstrates how to use the usePrec feature for evaluating segmentation accuracy by precision.
RunEvaluationAutoFlip.bat demonstrates how to use the autoFlip feature for automatically flipping segmentation labels.


---------
Requirements for re-compiling:
---------

 - Visual Studio 2013
 - Windows OS (The code uses Windows API for browsing directories).
 - OpenCV 3.1 (The current VS project settings refer to C:\opencv\build\.... for include and lib).
