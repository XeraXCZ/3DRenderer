#include <windows.h>
#include <math.h>
#include <stdio.h>

#define PI 3.14159265358979
#define TO_RADIANS(degrees) ((degrees) * PI / 180.0)
#define TO_DEGREES(radians) (radians * 180.0 / PI)

typedef struct CDS{
    float x,y,z;
}CDS;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND hwndg;

const int numVertices = 8;

//Default points of lines from which cube consists
CDS vertices[8] = {
    {0.0f, 0.0f, 0.0f}, // Front-bottom-left
    {2.0f, 0.0f, 0.0f},  // Front-bottom-right
    {2.0f, 2.0f, 0.0f},   // Front-top-right
    {0.0f, 2.0f, 0.0f},  // Front-top-left
    {0.0f, 0.0f, 2.0f},  // Back-bottom-left
    {2.0f, 0.0f, 2.0f},   // Back-bottom-right
    {2.0f, 2.0f, 2.0f},    // Back-top-right
    {0.0f, 2.0f, 2.0f},    // Back-top-left
};

//Definition of which points should be connected to form the lines of the cube
int cubeEdges[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Front face
    {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Back face
    {0, 4}, {1, 5}, {2, 6}, {3, 7},  // Connecting edges
};

CDS cam={-5.01f,-0.01f,1.01f};
float yaw=90;
float pitch=0; 
int screenWidth = 1920;
int screenHeight = 1080;
float fov=75;
//int m=670; //Depth factor used for tweaking camera distance
float d=670.0; //Distance of camera origin from canvas
float step=0.5;
int centerX, centerZ;
FILE * logs;

float GetVectorDistance(CDS v1,CDS v2){
    return sqrt(pow(v1.x-v2.x,2)+pow(v1.y-v2.y,2)+pow(v1.z-v2.z,2));
}

float GetPlanarDistance(float x1,float y1,float x2,float y2){
    return sqrt(pow(x1-x2,2)+pow(y1-y2,2));
}

/*
Returns one component of coordinates
Using a and b purposefully instead of x and y because we dont want to confuse with axis names
*/
int getCord(int offset,float a,float b,float ca,float cb,float yw){
    float cameraDistance = d;
    //Here i use similar triangles formula
    //int result=(int)(cameraDistance*((a-ca)/fabs(b-cb)));
    //int result=(int)(cameraDistance*tan(asin((a-ca)/(GetPlanarDistance(a,b,ca,cb)))-TO_RADIANS(yw)));
    int result=(int)(cameraDistance*tan(atan((a-ca)/(b-cb))-TO_RADIANS(yw)));
    //Log purpose only
    fprintf(logs,"%d = %fm*tan(%f-%f)=m*tan(atan(%f)-%f)=m*tan(atan((%.3f)/(%.3f))-%f)\n",result,cameraDistance,TO_DEGREES(atan((a-ca)/(b-cb))),yw,(a-ca)/(b-cb),TO_RADIANS(yw),a-ca,b-cb,TO_RADIANS(yw));
    return result+offset;
}

POINT getCord2(CDS a,CDS c){
    POINT p;
    p.x=centerX+(int)(d*tan(atan((a.x-c.x)/(a.y-c.y))-TO_RADIANS(yaw)));
    p.y=centerZ+(int)(d*tan(atan((a.z-c.z)/(((yaw/90)*(a.x-a.y)+a.y)-((yaw/90)*(c.x-c.y)+c.y)))-TO_RADIANS(pitch)));
    return p;
    
}

/*
Render whole cube
Important is position of camera. We calculate position of the cube relative to the camera (camera is the one who moves and cube is static)
The loop goes through all edges and gets new coordinates for the edge
*/
CDS * getCube(CDS position){
    CDS *arr=(CDS*)malloc(sizeof(CDS)*8);
    for(int i=0;i<8;i++){
        arr[i].x=vertices[i].x+position.x;
        arr[i].y=vertices[i].y+position.y;
        arr[i].z=vertices[i].z+position.z;
    }
    return arr;
}

int drawCube(HDC hdc,CDS position){
    CDS * vert=getCube(position);
    POINT p[]={{0,0},{0,0}};
    for (int i = 0; i < 12; i++) {
        int startIdx = cubeEdges[i][0];
        int endIdx = cubeEdges[i][1];
        CDS start=vert[startIdx],end=vert[endIdx];
        //if(start.y-cam.y<0||end.y-cam.y<0)continue;
        p[0].x = getCord(centerX,start.x,start.y,cam.x,cam.y,yaw);
        p[0].y = getCord(centerZ,start.z,start.y,cam.z,cam.y,pitch);
        p[1].x = getCord(centerX,end.x,end.y,cam.x,cam.y,yaw);
        p[1].y = getCord(centerZ,end.z,end.y,cam.z,cam.y,pitch);
        fprintf(logs,"\n");
        Polygon(hdc,p,2);
    } 
    free(vert);
    fprintf(logs,"\n");
}

int drawCube2(HDC hdc,CDS position){
    CDS * vert=getCube(position);
    POINT p[]={{0,0},{0,0}};
    for (int i = 0; i < 12; i++) {
        int startIdx = cubeEdges[i][0];
        int endIdx = cubeEdges[i][1];
        CDS start=vert[startIdx],end=vert[endIdx];
        //if(start.y-cam.y<0||end.y-cam.y<0)continue;
        p[0] = getCord2(start,cam);
        p[1] = getCord2(end,cam);
        fprintf(logs,"\n");
        Polygon(hdc,p,2);
    } 
    free(vert);
    fprintf(logs,"\n");
}

void ProjectAndDraw(HDC hdc){
    for(int i=0;i<10;i+=2)
        drawCube2(hdc,(CDS){i,0,0});
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int cmdShow)
{
    static TCHAR appName[] = TEXT("RotatingCube");
    HWND hwnd;
    MSG msg;
    WNDCLASS wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = appName;

    if (!RegisterClass(&wndclass)) {
        MessageBox(NULL, TEXT("Could not register Window Class!"), appName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(appName, TEXT("Rotating 3D Cube"),
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                        screenWidth, screenHeight, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, cmdShow);
    UpdateWindow(hwnd);

    centerX = screenWidth / 2;
    centerZ = screenHeight / 2;
    // Set the timer to update the angle and force a repaint

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// Window Procedure to handle events like painting and key presses
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;
    hwndg=hwnd;
    switch (message)
    {
    case WM_CREATE:
        logs=fopen("log.txt","w");
        if(logs==NULL){
            PostQuitMessage(-1);
            return -1;
        }
        d=(screenWidth/2)*tan(TO_RADIANS(fov/2));
        /*transform[0][0]=1/(ar-tan(fov/2));
        transform[1][1]=1/tan(fov/2);
        transform[2][2]=-1*((end+start)/(end-start));
        transform[2][3]=((-2*end*start)/(end-start));*/
        break;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rect);
        // Rotate the cube
        //RotateCube();
        // Draw the rotated cube
        ProjectAndDraw(hdc);
        rect.bottom /= 2;
        char temp[1000];
        sprintf(temp,TEXT("Yaw: %f\nX: %f\nY: %f\nZ: %f"),yaw,cam.x,cam.y,cam.z);
        DrawText(hdc, temp, -1, &rect, 0);
        EndPaint(hwnd, &ps);
        
        //MessageBox(hwnd, "OK", TEXT("Key Pressed"), MB_OK);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        switch(wParam){
            case 'W':cam.y+=step*cos(TO_RADIANS(yaw));cam.x+=step*sin(TO_RADIANS(yaw));break;
            case 'S':cam.y-=step*cos(TO_RADIANS(yaw));cam.x-=step*sin(TO_RADIANS(yaw));break;
            case 'A':cam.y-=step*cos(TO_RADIANS(yaw+90));cam.x-=step*sin(TO_RADIANS(yaw+90));break;
            case 'D':cam.y+=step*cos(TO_RADIANS(yaw+90));cam.x+=step*sin(TO_RADIANS(yaw+90));break;
            case VK_CONTROL:cam.z+=step;break;
            case VK_SPACE:cam.z-=step;break;
            case 'Q':yaw--;break;
            case 'E':yaw++;break;
            case 'P':d*=10;break;
            case 'O':d/=10;break;
            case 'K':yaw=0;break;
            case 'L':yaw=90;break;
            case VK_ESCAPE:        
                PostQuitMessage(0);
            default:break;
        }
        if(yaw>180||yaw<-180)yaw*=-1;
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

        case WM_SIZE:
            centerX = screenWidth / 2;
            centerZ = screenHeight / 2;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}
