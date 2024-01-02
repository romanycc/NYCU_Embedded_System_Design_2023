#include <fcntl.h> 
#include <fstream>
#include <linux/fb.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/ioctl.h>
#include <iostream>

struct framebuffer_info
{
    uint32_t bits_per_pixel;    // framebuffer depth
    uint32_t xres_virtual;      // how many pixel in a row in virtual screen
};

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path);

int main(int argc, const char *argv[])
{
    cv::Mat image;
    cv::Size2f image_size;
    
    framebuffer_info fb_info = get_framebuffer_info("/dev/fb0");
    std::ofstream ofs("/dev/fb0");

    // read image file (sample.bmp) from opencv libs.
    // https://docs.opencv.org/3.4.7/d4/da8/group__imgcodecs.html#ga288b8b3da0892bd651fce07b3bbd3a56
    printf("sss");
    image = cv::imread("advance.png", cv::IMREAD_COLOR);
    
    // get image size of the image.
    // https://docs.opencv.org/3.4.7/d3/d63/classcv_1_1Mat.html#a146f8e8dda07d1365a575ab83d9828d1
    image_size = image.size();
    printf("1");
    std::cout<<image_size;
    if (image.empty()) {
	    std::cerr << "Error: Could not read the image file." << std::endl;
	    return -1;
    }
    // transfer color space from BGR to BGR565 (16-bit image) to fit the requirement of the LCD
    // https://docs.opencv.org/3.4.7/d8/d01/group__imgproc__color__conversions.html#ga397ae87e1288a81d2363b61574eb8cab
    // https://docs.opencv.org/3.4.7/d8/d01/group__imgproc__color__conversions.html#ga4e0972be5de079fed4e3a10e24ef5ef0
    cv::Mat image_bgr565;     
    printf("2");
    cv::cvtColor(image, image_bgr565, cv::COLOR_BGR2BGR565);
    printf("3");
    // output to framebufer row by row


    for (int y = 0; y < image_size.height; y++)
    {
        // move to the next written position of output device framebuffer by "std::ostream::seekp()".
        // posisiotn can be calcluated by "y", "fb_info.xres_virtual", and "fb_info.bits_per_pixel".
        // http://www.cplusplus.com/reference/ostream/ostream/seekp/
        // move to the next written position of the output device framebuffer
        int position = y * fb_info.xres_virtual * fb_info.bits_per_pixel / 8;
        ofs.seekp(position);

        // write to the framebuffer
        ofs.write(reinterpret_cast<char*>(image_bgr565.ptr(y)), image_size.width * 2); // 2 bytes per pixel (BGR565)

        // write to the framebuffer by "std::ostream::write()".
        // you could use "cv::Mat::ptr()" to get the pointer of the corresponding row.
        // you also have to count how many bytes to write to the buffer
        // http://www.cplusplus.com/reference/ostream/ostream/write/
        // https://docs.opencv.org/3.4.7/d3/d63/classcv_1_1Mat.html#a13acd320291229615ef15f96ff1ff738
    }
    return 0;
}

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path)
{
    struct framebuffer_info fb_info;        // Used to return the required attrs.
    struct fb_var_screeninfo screen_info;   // Used to get attributes of the device from OS kernel.

    // open device with linux system call "open()"
    // https://man7.org/linux/man-pages/man2/open.2.html
    int fb_fd = open(framebuffer_device_path, O_RDWR); // read/write
    if (fb_fd == -1)
    {
        perror("Error opening framebuffer device");
        exit(1);
    }
    // get attributes of the framebuffer device thorugh linux system call "ioctl()".
    // the command you would need is "FBIOGET_VSCREENINFO"
    // https://man7.org/linux/man-pages/man2/ioctl.2.html
    // https://www.kernel.org/doc/Documentation/fb/api.txt
    // Usually, on success zero is returned.
    // Framebuffer Input/Output GET Variable SCREEN INFO
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &screen_info)) 
    {
        perror("Error reading framebuffer info");
        exit(1);
    }

    // put the required attributes in variable "fb_info" you found with "ioctl() and return it."
    // fb_info.xres_virtual =       // 8
    // fb_info.bits_per_pixel =     // 16
    fb_info.xres_virtual = screen_info.xres_virtual;
    fb_info.bits_per_pixel = screen_info.bits_per_pixel;
    return fb_info;
};
