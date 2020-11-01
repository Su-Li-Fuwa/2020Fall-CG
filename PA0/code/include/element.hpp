#pragma once
#include <iostream>
#include <image.hpp>
#include <algorithm>
#include <queue>
#include <cstdio>

class Element {
public:
    virtual void draw(Image &img) = 0;
    virtual ~Element() = default;
};

class Line : public Element {

public:
    int xA, yA;
    int xB, yB;
    Vector3f color;
    void draw(Image &img) override {
        BresenhamAlgo(img);
        printf("Draw a line from (%d, %d) to (%d, %d) using color (%f, %f, %f)\n", xA, yA, xB, yB,
                color.x(), color.y(), color.z());
    }
    void BresenhamAlgo(Image &img){
        if (xB != xA){
            int dx,dy,e,xs,xe,ys,ye,Delta;
            double f = double(yB-yA)/double(xB-xA);
            abs(f);
            if (f <= 1 and f >= -1){
                if (f>=0){
                    xs = std::min(xA,xB); xe = std::max(xA,xB); dx = xe-xs; Delta = 1;
                    ys = std::min(yA,yB); ye = std::max(yA,yB); dy = ye-ys; e = -dx;
                }
                else{
                    xs = std::max(xA,xB); xe = std::min(xA,xB); dx = xs-xe; Delta = -1;
                    ys = std::min(yA,yB); ye = std::max(yA,yB); dy = ye-ys; e = -dx;
                }
                for (int i=0;i<=dx;i++){
                    img.SetPixel(xs,ys,color);
                    xs += Delta; e += 2*dy;
                    if (e>=0)  {ys += 1; e -= 2*dx;}
                }
            }
            
            else{
                if (f>=0){
                    ys = std::min(yA,yB); ye = std::max(yA,yB); dy = ye-ys; Delta = 1;
                    xs = std::min(xA,xB); xe = std::max(xA,xB); dx = xe-xs; e = -dy;
                    
                }
                else{
                    ys = std::max(yA,yB); ye = std::min(yA,yB); dy = ys-ye; Delta = -1;
                    xs = std::min(xA,xB); xe = std::max(xA,xB); dx = xe-xs; e = -dy;
                }
                for (int i=0;i<=dy;i++){
                    img.SetPixel(xs,ys,color);
                    ys += Delta; e += 2*dx;
                    if (e>=0)  {xs += 1; e -= 2*dy;}
                }
            }
            
        }
        else
            for (int ys = std::min(yA,yB), ye = std::max(yA,yB); ys<=ye; ys++)
                img.SetPixel(xA,ys,color);
    }
};

class Circle : public Element {

public:
    int cx, cy;
    int radius;
    Vector3f color;
    void draw(Image &img) override {
        MidpointDraw(img);
        printf("Draw a circle with center (%d, %d) and radius %d using color (%f, %f, %f)\n", cx, cy, radius,
               color.x(), color.y(), color.z());
    }

    void boundaryJudge(int x, int y, Image &img){
        if (x>=0 and y>=0 and x<=img.Width()-1 and y<=img.Height()-1)
            img.SetPixel(x, y, color);
    }

    void MidpointDraw(Image &img){
        int x=0, y=radius, d=5-4*radius;
        boundaryJudge(cx+x,cy+y,img), boundaryJudge(cx+y,cy+x,img), boundaryJudge(cx-x,cy+y,img), boundaryJudge(cx-y,cy+x,img);
        boundaryJudge(cx+x,cy-y,img), boundaryJudge(cx+y,cy-x,img), boundaryJudge(cx-x,cy-y,img), boundaryJudge(cx-y,cy-x,img);
        while(x<=y){
            if (d<0)    d += 8*x + 12;
            else{
                d += 8*(x-y) + 20;
                y -= 1;
            }
            x += 1;
            boundaryJudge(cx+x,cy+y,img), boundaryJudge(cx+y,cy+x,img), boundaryJudge(cx-x,cy+y,img), boundaryJudge(cx-y,cy+x,img);
            boundaryJudge(cx+x,cy-y,img), boundaryJudge(cx+y,cy-x,img), boundaryJudge(cx-x,cy-y,img), boundaryJudge(cx-y,cy-x,img);
        }
    }
};

class Fill : public Element {

public:
    int cx, cy;
    Vector3f color, old_color;
    void draw(Image &img) override {
        // TODO: Flood fill
        old_color = img.GetPixel(cx,cy);
        // FloodFill4(img,cx,cy);
        ScanLineFill4(img,cx,cy);
        printf("Flood fill source point = (%d, %d) using color (%f, %f, %f)\n", cx, cy,
                color.x(), color.y(), color.z());
    }

    void FloodFill4(Image &img, int x, int y){
        if((img.GetPixel(x,y).x() == old_color.x()) and (img.GetPixel(x,y).y() == old_color.y()) and (img.GetPixel(x,y).z() == old_color.z())){
            img.SetPixel(x,y,color);
            //printf("%d %d\n",x,y);
            if (y+1 <= img.Height()-1) FloodFill4(img, x, y+1); 
            if (x+1 <= img.Width()-1) FloodFill4(img, x+1, y); 
            if (y-1 >= 0) FloodFill4(img, x, y-1); 
            if (x-1 >= 0) FloodFill4(img, x-1, y);
        }
    }

    bool isOldco(Image &img, int a, int b){ 
        return (img.GetPixel(a,b).x() == old_color.x()) and (img.GetPixel(a,b).y() == old_color.y()) and (img.GetPixel(a,b).z() == old_color.z());
    }

    void ScanLineFill4(Image &img, int x, int y){
        struct Seed{
            int x;
            int y;
        };

        struct Stackp{
            Seed data;
            Stackp* front;
        };

        int xl,xr,i,Num=0;
        bool spanNeedFill;
        Stackp Head, sSeed={{x,y},&Head};
        Stackp *tail,*head, *tmp;
        head = &Head;
        tail = &sSeed;
        Num += 1;

        
        while (Num!=0){
            x = tail->data.x, y=tail->data.y;
            while(isOldco(img, x, y) and x <= img.Width()-1){
                img.SetPixel(x,y,color);
                x += 1;
            }
            xr = x-1, x = tail->data.x - 1;
            // pop
            tmp = tail->front;
            if (Num != 1)
                delete tail;
            tail = tmp;
            Num -= 1;

            while(isOldco(img, x, y) and x >= 0){
                img.SetPixel(x,y,color);
                x -= 1;
            }
            xl = x+1;

            x=xl, y=y+1;
            if (y<=img.Height()-1)
                while(x <= xr){
                    spanNeedFill = false;
                    while(isOldco(img, x, y) and x <= xr){
                        spanNeedFill = true;
                        x += 1;
                    }
                    if (spanNeedFill){
                        Stackp *tmp = new Stackp;
                        Num += 1;
                        tmp->data = {x-1, y};
                        tmp->front = tail;
                        tail = tmp;
                        spanNeedFill = false;
                    }
                    while (not(isOldco(img, x, y)) and x <= xr) x += 1;
                }

            x=xl, y=y-2;
            if (y>=0)
                while(x <= xr){
                    spanNeedFill = false;
                    while(isOldco(img, x, y) and x <= xr){
                        spanNeedFill = true;
                        x += 1;
                    }
                    if (spanNeedFill){
                        Stackp *tmp = new Stackp;
                        Num += 1;
                        tmp->data = {x-1, y};
                        tmp->front = tail;
                        tail = tmp;
                        spanNeedFill = false;
                    }
                    while (not(isOldco(img, x, y)) and x <= xr) x += 1;
                }
        }
    }
};