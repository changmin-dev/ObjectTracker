#include "ObjectTracker.h"

int main(){
    VideoCapture cap;
    cap.open(0);

    if(cap.isOpened() == false){
        cout << "can not open cam" << endl;
        return -1;
    }

    Mat frame;
    objectTracker obT;
    obT.initWindow();

    while (1){
        cap >> frame;
        obT.getImage(frame);
        obT.trackObject();
        obT.showWindow();
        char c = (char)waitKey(10);
        if (c == 27)//ESC
            break;
    }
    return 0;
}