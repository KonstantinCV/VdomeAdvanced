#pragma once
#include "ofMain.h"
namespace vd {

class Render {

public:
    Render();
    void setup();
    void loadXML(ofXml &xml);
    void saveXML(ofXml &xml);

    bool getVSync();
    void setVSync(bool val);

    int getFrameRate();
    void setFrameRate(int val);

private:
    bool vsync;
    int framerate;
};

}
