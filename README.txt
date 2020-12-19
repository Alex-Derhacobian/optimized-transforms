=========================
| DAWN LAB STARTER TASK | 
=========================

Author: Alex Derhacobian
Date: 18 December 2020

Overview: In this starter task, I compiled OpenCV+contrib from source using CMake. The compile logs from the procress are stored in threeseparate txt files titled: 
 - opencv_contrib_compile_output.txt
 - opencv_c:wqontrib_compile_generate_output.txt
 - opencv_contrib_compile_nojava_nopython_output.txt

Each of these files are from three different compilings of OpenCV. The first was OpenCV alone. The second was when OpenCV was linked with the contrib. Additionally in this step, optionalities for DNNs, Java, and Python were disabled to provide an incremental improvement in runtime. These documents can be found in the folder titled "compile_logs". Once OpenCV was compiled, I linked the paths to OpenCV to the XCode Header Search Paths so that I could easily access OpenCV in XCode. 

Image: The image I used was "emperorpenguin.jpg". 

I've also created a Makefile. The "main.cpp" file contains all the functionalities of my program. 

The Code: In this program, I reimplement the Resize, CenterCropping, ToTensor, and normalize transforms from the Pytorch Library. I took into account the following functionalities of each of these transforms in the Pytorch library:  
	- Resize: 
		Valid Inputs: 
			<h> <w>: Will resize the height of the image to h and the width to w. 
			<n>: When a single number is inputed, then the smaller edge will be matched to this number. 
	- CenterCropping: 
		Valid Inputs: 
			<h> <w>: Crops the image relative to the center to a new image of height <h> and width <w>. 
			<n>: When a single number is inputed, then the operation is considered <h> = <w> = <n>. 
	-ToTensor: The Pytorch version converts the image into a (m x n x c) dimensional tensor. In my code, I implemented this function such the function returns an (m x n x 3) tensor. I assume that there are three color channels. I return the tensor as a vector of vectors representing the rows and columns of the image for each of the three color channnels (ie. RGB). 
	- Normalize: Similar to the Pytorch implementation, I normalize the image in a given range. The range is specificied in the source code, where I created global variables for the upper and lower bounds of the normalization. 


Optimization Techniques: I used two approaches to optimization. The first one was simply using the built-in optimization functions provided in the OpenCV library. The second more important method was considering the most optimal sequence of operations. First, I considered what are valid orderings of the operations. It doesn't make any sense to switch the order of tensorizing or normalizing, since those are operations that you perform downstream and rely on a fixed input image. Tensorizing, then resizing, then tensorizing is inefficent, so we assume that tensorizing and normalizing only happen as final commands.

My optimization is focused on the ordering of the Resize and CenterCrop functions. I realized that if we are resizing something, the center-cropped portion of the image is much more computationally inexpensive to compute beforeresizing. This is because if we are center-cropping on a smaller portion of the image, then if we perform a resizing on this smaller portion, it will achieve the same result but we are able to perform the resize command on a smaller area. Therefore, I implemented a method that adjusts the parameters passed into the center-cropping function such that they are equivanelt to a "Resize x CenterCrop" operation. I did this by dividing dimensions of the resizing by the differences D_1, D_2, where D_1= resize_height/original_height and D_2 = resize_width/original_width. 

The latencies of various operations were calculated and stored in a txt file titled "latency_outputs.txt". The latency per-operation was reduced significantly in some cases as a result of this optimization method. Note that in some cases the overall runtime of the optimized version is greater than the naive version since my parsing of the command-line arguments was probably not very efficient. But even in these cases, the per-case latency is higher in the naive case. One thing to note is that Resize and CenterCropping were outputted in reverse order in the optimized case because we performed the center cropping first. Latency was measured in milliseconds and microseconds per image. I ensured that latency was measured correctly by only logging the times for the specific functions and not including any other variable declarations, assignments, or function calls. This guaranteed that it was only measuring the runtime of each operation. The overall time of each approach was measured over all operations. 

Throughput: I recently got a new computer that is running on macOS Catalina, and valgrind is notorious for not working well with Catalina. I tried to install valgrind from source and use a valgrind alternative, but none were fruitful. I was also running everything locally. So in the case that I had the ability to run on a machine, I would use valgrind to track the number of bits used in each operation. I would log the number of bits used per second by each transform. The throughput would be measured in bits/second. Please let me know if I should clarify how I would calculate throughput. My machine just wasn't able to get valgrind running, and I didn't see any other way to keep track of bit-level operations performed by my program other than by using valgrind. 

 
Some confounding variables that affected the latency of the program were probably the inefficiency of my argument sorting code. An additional confounding variable that would affect the latency would be the specific build settings that I was using. I configured my Makefile with the -o3 flag to keep results accurate but also speed up operations. The latency could have also been affected by other applications running on parallel on my computer.
