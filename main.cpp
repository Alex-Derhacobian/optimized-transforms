// Filename: main.cpp
// Topic: DAWN Lab Application Code
// Administered by Mr. Daniel Kang (ddkang@cs.stanford.edu)
//
//  Created by Alex Derhacobian on 12/18/20
//
// DESCRIPTION: This program performs four basic operations on an input image: Resizing, CenterCropping, ToTensor, and Normalize. First we implement a naive version of all of these operations using OpenCV. Then, we optimize the sequence of operations in an optimized solution. Two main methods are used in optimization: 1) adjusting the order of the operational sequence, and 2) leveraging built-in OpenCV optimization. See the README.txt for more information about the optimization process

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/highgui.hpp>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <unordered_set>
#include "opencv2/imgproc.hpp"

using namespace std;
using namespace cv;

#define PATH "/Users/alexderhacobian/Desktop/emperorpenguin.jpg"//Replace with path to your own image
#define LOWER_BOUND_NORMAL 0 //lower bound of normalization
#define UPPER_BOUND_NORMAL 255 //upper bound of normalization

/* CLASS: Timer
 * Descprition: Used for keeping track of runtimes and calculating latency
 */
class Timer {
public:
    Timer()
    {
        m_StartTimepoint = std::chrono::high_resolution_clock::now();
    }
    ~Timer()
    {
        Stop();
    }
    void Stop()
    {
        auto endTimepoint = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
        auto end =std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
        auto duration = end - start;
        double ms = duration *0.001;
        
        std::cout << "Latency: " << duration << " microseconds/image (" << ms << " milliseconds/image)\n"; //output for latency calculations
        
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
};

/* FUNCTION: resize_img
 * Description: Resizes a <Mat image> to have size dim1 by dim2. In the case where only one command-line argument is passed, the main function sets dim1 and dim2 equal and performs the necessary scaling operations used in the Pytorch implementation
 * Returns: Resized image as Mat.
 */
Mat resize_img(Mat image, int dim1, int dim2) {
    if (image.empty()) { //error handling
        cout << "Error: Image not found or cannot be opened" <<std::endl;
        return image;
    }
    cv::resize(image, image, cv::Size(dim2, dim1));
    return image;
}

/* FUNCTION: optimized_resize
 * Description: Adds OpenCV optimization function to speed up task. Resizes <Mat image> to size <dim1> by <dim2>
 * Returns: Resized image as Mat.
 */
Mat optimized_resize(Mat image, int dim1, int dim2) {
    useOptimized();
    return resize_img(image, dim1, dim2);
}

/* FUNCTION: centerCrop
 * Description: Crops a <Mat image> to have size dim1 by dim2 relative to the centerpoint. In the case where only one command-line argument is passed, the main function sets dim1 and dim2 equal and performs a square centerCropping as in the Pytorch implementation
 * Returns: Cropped image as Mat.
 * References used: gist.github.com/1duo/c868f0bccf0d1f6cd8b36086ba295e04
 */
Mat centerCrop(Mat image, const int dim1, int dim2) {
    if (image.empty()) {//error handling
        cout <<  "Could not open or find the image" << std::endl;
        return image;
    }
    const int offsetW = (image.cols - dim2) / 2;
    const int offsetH = (image.rows - dim1) / 2;
    const Rect roi(offsetW, offsetH, dim2, dim1);
    image = image(roi).clone();
    return image;
}

/* FUNCTION: optimized_center_crop
 * Description: Adds OpenCV optimization function to speed up task. Centercrops <Mat image> to size <dim1> by <dim2> relative to centerpoint
 * Returns: Resized image as Mat.
 */
Mat optimized_center_crop(Mat image, int dim1, int dim2) {
    useOptimized();
    return centerCrop(image, dim1, dim2);
}

/* FUNCTION: ToTensor
 * Description: Produces an (m x n x 3) tensor in the form a vector of vectors of vectors of integers for a given <Mat image> of size (m x n) with 3 color channels. In this implimentation, we assume there are 3 color channels for the input image.
 * Returns: (m x n x 3) tensor of type vector<vector<vector<int>>>
 */
vector<vector<vector<int>>> ToTensor(Mat image) {
    vector<vector<vector<int>>> tensor;
    for (int c = 0; c < 3; c++) {
        vector<vector<int>> cur_color;
        for (int y = 0; y < image.rows; y++) {
            vector<int> cur_row;
            for (int x = 0; x < image.cols; x++) {
                cur_row.push_back(image.at<cv::Vec3b>(y,x)[c]);
            }
            cur_color.push_back(cur_row);
        }
        tensor.push_back(cur_color);
    }
    return tensor;
}

/* FUNCTION: optimized_to_tensor
 * Description: Adds OpenCV optimization function to speed up task. Converts the image to a (m x n x 3) tensor using ToTensor
 * Returns: Tensorized image.
 */
vector<vector<vector<int>>> optimized_to_tensor(Mat image) {
    useOptimized();
    return ToTensor(image);
}

/* FUNCTION: normalizer
 * Description: normalizes the input image
 * Returns: normalized input image.
 */
Mat normalizer(Mat image) {
    Mat imgfile_normalized;
    cv::normalize(image, imgfile_normalized, UPPER_BOUND_NORMAL, LOWER_BOUND_NORMAL, NORM_MINMAX,-1, noArray());
    return imgfile_normalized;
}

/* FUNCTION: optimized_normalizer
 * Description: optimizes normalizer
 * Returns: normalized input image.
 */
Mat optimized_normalizer(Mat image) {
    useOptimized();
    return normalizer(image);
}


int main(int argc, const char * argv[]) {
    Mat image;
    image = imread(PATH);
    std::unordered_set<string> operations = {"--resize", "--centercrop", "--tensor", "--normalize"};
    auto naive_start = std::chrono::high_resolution_clock::now(); //clock for counting total runtime for naive method
    //NAIVE CASES HANDLED FIRST
    for (int i = 0; i < argc; i++) {
        if(operations.find(argv[i])!=operations.end()) { //see if current argument is a valid operation
            if (strcmp(argv[i], "--resize")==0) {//handling resize
                    if ((i+2)<argc) {
                        if (operations.find(argv[i+2])!=operations.end()) { //if there is only one parameter passed in
                            if (image.rows > image.cols) {
                                int new_w = std::stoi(argv[i+1]);
                                {Timer timer; image = resize_img(image, new_w*image.rows/image.cols, new_w);};
                                i+=1;
                                continue;
                            }
                            else {
                                int new_h = std::stoi(argv[i+1]);
                                {Timer timer; image = resize_img(image, new_h, new_h*image.cols/image.rows);};
                                i+=1;
                                continue;
                            }
                        }
                        else { //if there are two parameters passed in
                            {Timer timer; image = resize_img(image, std::stoi(argv[i+1]),std::stoi(argv[i+2]));};
                            i+=2;
                            continue;
                        }
                    }
                    else {//only one parameter passed in with no other proceeding transforms
                        if (image.rows > image.cols) {
                            int new_w = std::stoi(argv[i+1]);
                            {Timer timer; image = resize_img(image, new_w*image.rows/image.cols, new_w);};
                            i+=1;
                            continue;
                        }
                        else {
                            int new_h = std::stoi(argv[i+1]);
                            {Timer timer; image = resize_img(image, new_h, new_h*image.rows/image.cols);};
                            i+=1;
                            continue;
                        }
                    }
                    
                }
                else {//all other cases
                    if (strcmp(argv[i], "--centercrop")==0) {
                        if ((i+2)<argc && operations.find(argv[i+2])==operations.end()) {
                            int new_w = std::stoi(argv[i+2]);
                            int new_h = std::stoi(argv[i+1]);
                            {Timer timer; image = centerCrop(image, new_h, new_w);};
                            i+=2;
                            continue;
                        }
                        else {
                            int new_vals = std::stoi(argv[i+1]);
                            {Timer timer; image = centerCrop(image, new_vals, new_vals);};
                            i+=1;
                            continue;
                        }
                    }
                    else if (strcmp(argv[i], "--tensor")==0) {
                        {Timer timer; ToTensor(image);
                        }
                    }
                    else {
                        {Timer timer; normalizer(image);}
                    }
                }
        }
    }
    auto naive_stop = std::chrono::high_resolution_clock::now();
    auto naive_duration = std::chrono::duration_cast<std::chrono::microseconds>(naive_stop - naive_start);
    cout << "Total Execution Time of Naive Solution: " << naive_duration.count()*0.001 << " milliseconds" << endl;
    
    auto improved_start = std::chrono::high_resolution_clock::now();//starting clock for optimized cases
    //Optimized sequence of operations
    for (int i = 0; i < argc; i++) {
        if(operations.find(argv[i])!=operations.end()) {
            if (strcmp(argv[i], "--resize")==0) {
                if (((i+3)<argc) && ((strcmp(argv[i+3], "--centercrop")==0) || (strcmp(argv[i+2], "--centercrop")==0))) {//see if centercropping will be done
                    if (strcmp(argv[i+3], "--centercrop")==0) {//that thre are two parameters for resize
                        int new_h = std::stoi(argv[i+1]);
                        int new_w = std::stoi(argv[i+2]);
                        double dh = double(new_h)/double(image.rows);
                        double dw = double(new_w)/double(image.cols);
                        if (i+5<argc && operations.find(argv[i+5])==operations.end()) {
                            int new_h = std::stoi(argv[i+4])/dh;
                            int new_w = std::stoi(argv[i+5])/dw;
                            {Timer timer; image = optimized_center_crop(image, new_h, new_w);};
                            i+=5;
                        }
                        else {
                            int new_h = std::stoi(argv[i+4])/dh;
                            int new_w = std::stoi(argv[i+4])/dw;
                            {Timer timer; image = optimized_center_crop(image, new_h, new_w);};
                            i+=4;
                        }
                        {Timer timer; image = optimized_resize(image, image.rows*dh, image.cols*dw);};
                        continue;
                    }
                    else {//then there is only one parameter for resizing, so we need to perform to Pytorch-specific resizing operation for one parameter
                        if (image.rows > image.cols) {
                            int new_w = std::stoi(argv[i+1]);
                            int new_h = new_w;
                            double dh = double(new_h)/double(image.rows);
                            double dw = double(new_w)/double(image.cols);
                            if (operations.find(argv[i+3])!=operations.end() && operations.find(argv[i+4])!=operations.end()) {//now performing cropping
                                int new_h = std::stoi(argv[i+3])/dh;
                                int new_w = std::stoi(argv[i+4])/dw;
                                {Timer timer; image = optimized_center_crop(image, new_h, new_w);};
                                i+=4;
                            }
                            else {//cropping with one parameter
                                int new_h = std::stoi(argv[i+3])/dh;
                                int new_w = std::stoi(argv[i+3])/dw;
                                {Timer timer; image = optimized_center_crop(image, new_h, new_w);};
                                i+=3;
                            }
                            {Timer timer; image = optimized_resize(image, image.rows*dh, image.cols*dw);};
                            continue;
                        }
                        else {//the case where the image's longer side is its width
                            int new_h = std::stoi(argv[i+1]);
                            int new_w = new_h;
                            double dh = double(new_h)/double(image.rows);
                            double dw = double(new_w)/double(image.cols);
                            if (i+4<argc) {
                                int new_h = std::stoi(argv[i+3])/dh;
                                int new_w = std::stoi(argv[i+4])/dw;
                                {Timer timer; image = optimized_center_crop(image, new_h, new_w);};
                                i+=4;
                            }
                            else {
                                int new_h = std::stoi(argv[i+3])/dh;
                                int new_w = std::stoi(argv[i+3])/dw;
                                {Timer timer; image = optimized_center_crop(image, new_h, new_w);};
                                i+=3;
                            }
                            {Timer timer; image = optimized_resize(image, image.rows*dh, image.cols*dw);};
                            continue;
                        }
                    }
                }
                else {//no call for centercropping. This resizing method follows the same logic as above
                    if ((i+2)<argc) {
                        if (operations.find(argv[i+2])!=operations.end()) {
                            if (image.rows > image.cols) {
                                int new_w = std::stoi(argv[i+1]);
                                {Timer timer; image = optimized_resize(image, new_w*image.rows/image.cols, new_w);};
                                i+=1;
                                continue;
                            }
                            else {
                                int new_h = std::stoi(argv[i+1]);
                                {Timer timer; image = optimized_resize(image, new_h, new_h*image.cols/image.rows);};
                                i+=1;
                                continue;
                            }
                        }
                        else {
                            {Timer timer; image = optimized_resize(image, std::stoi(argv[i+1]),std::stoi(argv[i+2]));};
                            i+=2;
                            continue;
                        }
                    }
                    else {
                        if (image.rows > image.cols) {
                            int new_w = std::stoi(argv[i+1]);
                            {Timer timer; image = optimized_resize(image, new_w*image.rows/image.cols, new_w);};
                            i+=1;
                            continue;
                        }
                        else {
                            int new_h = std::stoi(argv[i+1]);
                            {Timer timer; image = optimized_resize(image, new_h, new_h*image.rows/image.cols);};
                            i+=1;
                            continue;
                        }
                    }
                    
                }
            }
            else {//separate cases of ceter cropping, tensorizing, and normalizing. The center cropping case follows the same logic as above
                if (strcmp(argv[i], "--centercrop")==0) {
                    if ((i+2)<argc && operations.find(argv[i+2])==operations.end()) {
                        int new_w = std::stoi(argv[i+2]);
                        int new_h = std::stoi(argv[i+1]);
                        {Timer timer; image = optimized_center_crop(image, new_h, new_w);};
                        i+=1;
                        continue;
                    }
                    else {
                        int new_vals = std::stoi(argv[i+1]);
                        {Timer timer; image = optimized_center_crop(image, new_vals, new_vals);};
                        i+=1;
                        continue;
                    }
                }
                else if (strcmp(argv[i], "--tensor")==0) {//tensorizing
                    {
                        Timer timer;
                        optimized_to_tensor(image);
                    }
                }
                else {//normalizing
                    {
                        Timer timer;
                        optimized_normalizer(image);
                    }
                }
            }
        }
    }
    auto improved_stop = std::chrono::high_resolution_clock::now();
    auto improved_duration = std::chrono::duration_cast<std::chrono::microseconds>(improved_stop - improved_start);
    cout << "Total Execution Time of Optimized Solution: " << improved_duration.count()*0.001 << " milliseconds" << endl;
    return 0;
}
