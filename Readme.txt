This evaluation kit provides evaluation and visualization tools for our dataset.
The code and dataset is provided for research use only.
If you use our dataset, please cite our CVPR 2016 paper.
We thank Daniel Scharstein for providing code for reading and writing .flo files.
We also thank to Lin et al. [25], Rubinstein et al. [42], Hariharan et al. [20]
for providing datasets from which the images of our dataset are taken.
Our dataset can be found at http://taniai.space/projects/cvpr16_dccs/

@InProceedings{Taniai2016,
	author = {Tatsunori Taniai and Sudipta N. Sinha and Yoichi Sato},
	title = {{Joint Recovery of Dense Correspondence and Cosegmentation in Two Images}},
	booktitle = {IEEE Conference on Computer Vision and Pattern Recognition (CVPR)},
	month = {June},
	year = {2016}
}

---------
Contents:
---------

- Dataset : Sample dataset containing some image pairs from FG3DCar, JODS and PASCAL.
- Results : Sample results for the image pairs
- EvalToolVC     : Evaluation and visualization tool written using Visual Studio 2013 (C++)
- EvalToolMATLAB : Evaluation and visualization tool written using MATLAB

-------------
Instructions:
-------------

The following files describe how to use the evaluation toolkit.

	- EvalToolVC/Readme.txt
	- EvalToolMATLAB/Readme.txt

------------------
Evaluation format:
------------------

To evaluate a new method, one should provide the following four files for all the image pairs in our full benchmark.
	- flow1.flo : Floating-point 2D flow map (from image1 to image2).
	- flow2.flo : Floating-point 2D flow map (from image2 to image1).
	- mask1.png : Foreground mask for image1
	- mask2.png : Foreground mask for image2

If the mask files are not present, then our evaluation code estimates them using bidirectional left-right consistency of the two flow maps (flow1.flo and flow2.flo) using a specific threshold.
If the flow files are not present, then the flow estimation scores are not reported.
If the image resolutions of these files do not match the original resolutions of image1 and image2, then the flow maps and mask images are resized to the original image resolutions.

----------------------
Segmentation accuracy:
----------------------

Given the estimated foreground mask and the ground truth foreground masks for each image, the segmentation accuracy is computed in terms of the intersection-over-union ratio metric.


--------------
Flow accuracy:
--------------

Given the estimated and true 2D flow maps, U(p) = [u(p), v(p)] and G(p) = [gu(p), gv(p)] for each image,
the endpoint errors are computed as E(p) = sqrt(|u(p)-gu(p)|^2 + |v(p)-gv(p)|^2) * scale
in a normalized scale where the larger dimension of the images is set to 100 pixels.
The flow accuracy is then computed as 1 - #{ p | p is in true foreground and E(p) is less than a threshold } / #{ p | p is in true foreground }.
We compute scores for a range of threshold values -- from 1 to 50 pixels.
The scores corresponding to the 5 pixel threshold are reported as the primary results.

--------------------
Executable Binaries:
--------------------

Windows binaries of EvalToolVC are avaialbe at https://github.com/t-taniai/TSS_CVPR2016_EvaluationKit/releases


--------
History:
--------

06/10/2016 v1.0 Released the evaluation kit
07/28/2016 v1.1 Added "autoFlip" option
08/26/2016 v1.2 Added more example results and fixed bat files of EvalToolVC. Released windows binaries using Github releases.
