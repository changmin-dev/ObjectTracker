#include "ObjectTracker.h"
#include <cstdlib>

objectTracker::objectTracker(){
    isSelectObject = false;	//처음에는 꺼진 상태
    trackObjectState = 0;	//추적하지 않는 상태

    //s,v값 지정(트랙바체크하고 고정값으로 트랙바 넣으면 느려짐)
    vmin = 200, vmax = 256;
    smin = 30;

    //히스토그램 파라미터 초기화
    hsize = 16;					// hue의 bin의 개수.
    hranges[0] = 0; hranges[1] = 180; //opencv는 h값이 0 ~ 180
    phranges = hranges;
    //오브젝트 위치
    objectPos = 'I';

    //미리 주어진 탬플릿의 히스토 그램을 구한다.
    getTemplateHistTpl();
}
void objectTracker::showWindow(){
    //선택영역을 역상으로 표시한다.
    if (isSelectObject && selection.width > 0 && selection.height > 0)		{
        Mat roi(image, selection);
        bitwise_not(roi, roi);
        firstSel = selection;
    }
    imshow("Show me the code", image);
}
void onMouse(int event, int x, int y, int, void* obj){

    objectTracker* obt = static_cast<objectTracker*>(obj);
    if (obt)
        obt->onMouseEvent(event, x, y);
}
void objectTracker::initWindow(int mode){
    namedWindow("Show me the code", CV_WINDOW_AUTOSIZE);
    setMouseCallback("Show me the code", onMouse, this);
    if (mode != 0){
        createTrackbar("Vmin", "Show me the code", &vmin, 256, 0);
        createTrackbar("Vmax", "Show me the code", &vmax, 256, 0);
        createTrackbar("smix", "Show me the code", &smin, 256, 0);
    }
}
void objectTracker::onMouseEvent(int event, int x, int y){

    if (isSelectObject){
        selection.x = MIN(x, origin.x);         selection.y = MIN(y, origin.y);
        selection.width = std::abs((double)(x - origin.x));         selection.height = std::abs((double)(y - origin.y));
        selection &= Rect(0, 0, image.cols, image.rows);		// 원본 영상의 크기와의 & 연산을 통해 선택된 크기가 원본 영상을 넘지 않게 한다.  마우스가 화면밖일때 대처
    }

    switch (event){
        case CV_EVENT_LBUTTONDOWN:
            origin = Point(x, y);				// 템플레이트의 왼쪽 상단 지점. 이 부분부터 드래그하여 영역을 선택한다.
            selection = Rect(x, y, 0, 0);		// 높이와 폭은 일단 0으로 설정한다.
            isSelectObject = true;			// 템플레이트 선택이 시작되었음을 알리는 플래그.
            break;
        case CV_EVENT_LBUTTONUP:
            isSelectObject = false;			// 템플레이트 선택은 끝났음을 알린다.
            if (selection.width > 0 && selection.height > 0)
                trackObjectState = -1;			// 템플레이트에 대한 히스토그램을 구할 필요가 있음을 의미. 타겟에 대해서 projection을 할 때는 1.
            break;
    }
}
void objectTracker::getTemplateHist(){//Camshift는 히스토그램에 대한 backproject을 이용해서 수행된다.

    //사전에 지정한 SV값에 대한 마스크를 저장
    inRange(hsv, Scalar(0, smin, MIN(vmin, vmax)), Scalar(180, 256, MAX(vmin, vmax)), mask);

    //hsv에서 휴값만 빼냄
    hue.create(hsv.size(), hsv.depth());
    int ch[] = { 0, 0 };
    mixChannels(&hsv, 1, &hue, 1, ch, 1);

    if (trackObjectState < 0)		// 마우스로 드래그하여 템플레이트 영역을 설정하면 이 값은 -1이 된다.
    {
        //Mat roi(hue, selection);			// hue 영상에서 선택한 부분만 오린 ROI 영상. 입력 영상의 hue 정보가 담겨 있는 영상에서 해당 영역만 자른 영상.
        //Mat maskroi(mask, selection);		// mask 영상(HSV가 모두 범위 안에 들은 이진 영상)에서 선택한 부분만 오린 ROI 영상.
        Mat roi(hue, selection);
        Mat maskroi(mask, selection);
        //히스토그램을 구하고 정규화 한다.
        calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
        normalize(hist, hist, 0, 255, CV_MINMAX);

        trackWindow = selection;		// 처음 시작이니 일단 템플레이트를 검출한 부분(selection)을 트래킹할 윈도로 할당한다.  trackWindow은 CamShift 함수의 파라미터이다.
        trackObjectState = 1;			// 트랙킹을 시작한다. 1이란 의미는 템플레이트에 대해서는 또 히스토그램을 구하지 않는다는 것이다.
    }
}
void objectTracker::getTemplateHistTpl(){
    Mat tpl = imread(TPL_DIR);
    if(tpl.empty()){
        cout << "Template is not found!" << endl;
        std::terminate();
    }

    Mat tpl_hue;
    Mat tpl_hsv;
    Mat tpl_mask;

    cvtColor(tpl, tpl_hsv, CV_BGR2HSV);

    inRange(tpl_hsv, Scalar(0, smin, MIN(vmin, vmax)), Scalar(180, 256, MAX(vmin, vmax)), tpl_mask);
    tpl_hue.create(tpl_hsv.size(), tpl_hsv.depth());
    int ch[] = { 0, 0 };
    mixChannels(&tpl_hsv, 1, &tpl_hue, 1, ch, 1);

    calcHist(&tpl_hue, 1, 0, tpl_mask, hist, 1, &hsize, &phranges);
    normalize(hist, hist, 0, 255, CV_MINMAX);
}

void objectTracker::trackObject(){//문제 발생
    cvtColor(image, hsv, CV_BGR2HSV);
    getTemplateHist();

    int tmp_channels[] = { 0 };
    cvtColor(image, hsv, CV_BGR2HSV);
    inRange(hsv, Scalar(0, smin, MIN(vmin, vmax)), Scalar(180, 256, MAX(vmin, vmax)), mask);
    hue.create(hsv.size(), hsv.depth());
    int ch[] = { 0, 0 };
    mixChannels(&hsv, 1, &hue, 1, ch, 1);

    calcBackProject(&hue, 1, tmp_channels, hist, backproj, &phranges);

    erode(backproj, backproj, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
    dilate(backproj, backproj, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

    dilate(backproj, backproj, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
    erode(backproj, backproj, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

    backproj &= mask;
    trackCamShift();
}
void objectTracker::trackObjectTpl(){

    //getTemplateHistTpl();//생성자에서 돌린다.

    int tmp_channels[] = { 0 };
    cvtColor(image, hsv, CV_BGR2HSV);
    inRange(hsv, Scalar(0, smin, MIN(vmin, vmax)), Scalar(180, 256, MAX(vmin, vmax)), mask);
    hue.create(hsv.size(), hsv.depth());
    int ch[] = { 0, 0 };
    mixChannels(&hsv, 1, &hue, 1, ch, 1);

    calcBackProject(&hue, 1, tmp_channels, hist, backproj, &phranges);

    erode(backproj, backproj, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
    dilate(backproj, backproj, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

    dilate(backproj, backproj, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
    erode(backproj, backproj, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

    backproj &= mask;

    trackCamShift();
}
void objectTracker::trackCamShift(){
    if (trackObjectState)//처음에는 히스토그램 정보가 없으므로 동작하지 않게 한다.(0)
    {
        RotatedRect trackBox;
        if (trackWindow.height <= 0 || trackWindow.width <= 0){
            putText(image, "TARGET LOST", Point(0, image.cols / 2), FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255));
            this->objectPos = 'f';//정지
        }
        else{
            //camShift로 추적
            trackBox = CamShift(backproj, trackWindow, TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));
            cout << trackBox.size << endl;
            int midWimage = image.cols / 2;
            int midHimage = image.rows / 2;

            int midWtpl = trackBox.center.x;
            int midHtpl = trackBox.center.y;

            if (midWimage - 40 > midWtpl){
                putText(image, "LEFT", Point(0, image.cols / 2), FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255));
                this->objectPos = 'L';
            }

            else if (midWimage + 40 < midWtpl){
                putText(image, "RIGHT", Point(0, image.cols / 2), FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255));
                this->objectPos = 'R';
            }
            else{
                //putText(image, "GOOD", Point(0, image.cols / 2), FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255));
                //this->objectPos = 'G';
                int selectionSize = (image.size().height * image.size().width)*0.15;//기준 크기 이미지의 15%
                int trackBoxSize = trackBox.boundingRect().height * trackBox.boundingRect().width;//트랙박스크기

                if (selectionSize > trackBoxSize){
                    this->objectPos = 'F';
                    putText(image, "FAR", Point(0, image.cols / 2), FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255));
                }
                else {
                    this->objectPos = 'B';
                    putText(image, "NEAR", Point(0, image.cols / 2), FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255));
                }

            }

        }//end camshift

        //윈도우가 너무 작아지면 다시 만들어 준다.(크래쉬 방지)
        if (trackWindow.area() <= 1)
        {
            int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5) / 6;
            trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                               trackWindow.x + r, trackWindow.y + r) &
                          Rect(0, 0, cols, rows);
        }

        //찾은 것을 그려준다.
        Rect brect = trackBox.boundingRect();
        rectangle(image, brect, Scalar(255, 0, 0));
    }
}
char objectTracker::getObjectPos(){
    return objectPos;
}