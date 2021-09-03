Sony TOF client project

This project use opencv 3.2.0, which was tested with Qt 5.9.0.
Most of opencv features may still work with other Qt version, but please avoid GUI function like imshow.

To setup opencv:
1. extract opencv.tar.gz to somewhere (eg: D:/opencv)
2. Create OPENCV_SDK_DIR environment variable, value is opencv extracted location
For more info, check SonyTOF_client.pro file to see how this SDK_OPENCV_DIR variable was used.

