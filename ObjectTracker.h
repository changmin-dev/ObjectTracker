//
// Created by Changmin Kim on 2017. 5. 7..
//

#ifndef OBJECT_TRACKER_OBJECTTRACKER_H
#define OBJECT_TRACKER_OBJECTTRACKER_H


#include <iostream>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;
#define TPL_DIR "markerTpl2.png"

//callback함수 맴버가 콜백으로 바로 들어가지 않으므로 이 함수를 거쳐서 맴버를 호출하게한다.
void onMouse(int event, int x, int y, int, void* obj);
class objectTracker{
private:
    Mat image;				//추적할 이미지
    int trackObjectState;	//추적여부
    Rect trackWindow;		// 추적된 타겟 영역 윈도.

    bool isSelectObject;	//마우스로 선택여부
    Point origin;			// 템플레이트의 왼쪽 상단 지점. 이 부분부터 드래그하여 영역을 선택한다.
    Rect selection;			// 템플레이트 영역. 사각형. 이 부분은 역상으로 표현된다.

    int	vmin, vmax;			//	value 값의 경계
    int	smin;				// 최소 채도 값. smin~255까지의 채도 값을 갖는 화소가 검출 대상이다.

    //원본영상을 hsv로 바꾸고 hue부분만 추출하고, SV부분 마스크를 얻고, 히스토그램을 구해서 backprojection한다.
    Mat hsv, hue, mask, hist, backproj;

    //히스토그램을 위한 값들
    int hsize;// hue의 bin의 개수.
    float hranges[2];//2채널만 사용
    const float* phranges;

    Rect firstSel;//처음 박스로 클릭한 부분
    /*
    // objectPos; 오브젝트의 위치를 나타냄
    L:Left
    R:Right
    F:Far
    B:Near(Back해야함)
    M:missing(정지)
    I:initial
    */
    char objectPos;

public:
    objectTracker();
    ~objectTracker(){
    }
    void getImage(const Mat& image){
        image.copyTo(this->image);
    }
    void getTemplateHist();

    void trackObject();
    void showWindow();
    void initWindow(int mode = 0);//0이외의 숫자가 나오면 v값의 범위를 나타낼수 있음
    void onMouseEvent(int event, int x, int y);
    friend void onMouse(int event, int x, int y, int, void* obj);
    char getObjectPos();
    void setTracking(int mode){
        trackObjectState = mode;
        trackWindow = Rect(1,1,image.rows,image.cols);
    }
    void trackCamShift();
    //템플릿으로 추적 - 임의의 파일
    void getTemplateHistTpl();//연구중
    void trackObjectTpl();
};


#endif //OBJECT_TRACKER_OBJECTTRACKER_H
