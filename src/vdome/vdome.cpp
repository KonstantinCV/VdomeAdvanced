#include "vdome.h"
#include "command.h"
#include "ofxMultiGLFWWindow.h"

namespace vd {

// global variables

float projCount = 1;
int winCount = 1;

int maxHistory = 25;
CommandHistory history;
vector<ofPixels> maskHistory;



/******************************************

 CONSTRUCTOR

 ********************************************/

vdome::vdome() {
    menu.input = &input;
    menu.model = &model;
    menu.windows = &windows;
    menu.projectors = &projectors;
    socket.input = &input;
    input.socket = &socket;
	input.model = &model;
    autosave = false;
	wIndex = 0;

    #ifdef TARGET_OSX
        cKey = OF_KEY_COMMAND;
    #else
        cKey = OF_KEY_CONTROL;
    #endif
}

/******************************************

 SETUP

 ********************************************/

void vdome::setup(){
    ofSetEscapeQuitsApp(false);

    glfw = (ofxMultiGLFWWindow*)ofGetWindowPtr();
    glfwWindows = &glfw->windows;

    render.setup();
    model.setup();

    // input setup
    input.source = 0;
    input.framerate = render.getFrameRate();
    //input.setup();

    // projection shader
	shader.load("settings/shaders/vdome.vert", "settings/shaders/vdome.frag");

    // xml settings
    xmlFile = "settings.xml";
    saveThread.xml.push_back(&xml);
    saveThread.files.push_back(xmlFile);

    if (xml.load(xmlFile))
        loadXML(xml);

	// hide console window
    #ifdef TARGET_WIN32
		#ifdef VDOME_DEBUG
		#else
	    HWND handleWindow;
        AllocConsole();
        handleWindow = FindWindowA("ConsoleWindowClass", NULL);
        ShowWindow(handleWindow, 0);
		#endif
	#endif

    for(int i=0; i<projCount; i++) {
        Projector p = projectors[i];
        saveThread.xml.push_back(p.plane.wXml);
        saveThread.files.push_back("settings/warp/warp-"+ofToString(i+1)+".xml");
        saveThread.image.push_back(projectors[i].mask.maskFboImage);
        saveThread.imageFiles.push_back("settings/masks/mask-" + ofToString(i+1) + ".png");
    }

    menu.autosave = autosave;
	glfw->showWindow(glfwWindows->at(0));
	menu.setup();
}

/******************************************

 UPDATE

 ********************************************/

void vdome::update() {
    if (wIndex == 0) {
        input.update();

        if (socket.enabled)
            socket.update();
    }

	for(int i=0; i<projectors.size(); i++) {
		projectors[i].update();
    }

    if (menu.active) {
        menu.update();
        if (saveThread.saved) {
            menu.saved = true;
            saveThread.saved = false;
        }
    }
}

/******************************************

 DRAW

 ********************************************/

void vdome::draw(){
    ofSetHexColor(0xFFFFFF);
	int wi = 0;
	wi = glfw->getWindowIndex();
	for (int i=windows[wi].firstProjector; i<=windows[wi].lastProjector; i++) {
        if (projectors[i].enable) {

            projectors[i].begin();
                input.bind();
                    model.draw();
                input.unbind();
            projectors[i].end();

            ofDisableNormalizedTexCoords();
            projectors[i].renderFbo.begin();
            ofClear(0);
            projectors[i].bind();

                projectors[i].draw();

            projectors[i].unbind();
            projectors[i].renderFbo.end();
            ofEnableNormalizedTexCoords();

            projectors[i].renderFbo.getTextureReference().bind();

                if (projectors[i].active)
                    projectors[i].mask.draw();

					shader.begin();

                    shader.setUniform1f("brightness", projectors[i].brightness);
                    shader.setUniform1f("contrast", projectors[i].contrast);
                    shader.setUniform1f("blackLevel", projectors[i].blackLevel);
                    shader.setUniform1f("whiteLevel", projectors[i].whiteLevel);

                    shader.setUniform1f("hue", projectors[i].hue);
                    shader.setUniform1f("saturation", projectors[i].saturation);
                    shader.setUniform1f("lightness", projectors[i].lightness);

                    shader.setUniform1f("gamma", projectors[i].gamma);
                    shader.setUniform1f("gammaR", projectors[i].gammaR);
                    shader.setUniform1f("gammaG", projectors[i].gammaG);
                    shader.setUniform1f("gammaB", projectors[i].gammaB);

					shader.setUniform1i("interp", 1 );
					shader.setUniform1f("amt", 1.0 );
					shader.setUniform1f("mapdim", 256.0 );

					shader.setUniformTexture("texsampler", projectors[i].renderFbo.getTextureReference(), 0);
					shader.setUniformTexture("colorlut", projectors[i].curves.colorlutTextureRef(), 1);
                    shader.setUniformTexture("maskTex", projectors[i].mask.maskFbo.getTextureReference(), 2);

                    projectors[i].renderPlane.draw();

                shader.end();

            projectors[i].renderFbo.getTextureReference().unbind();

        }

        if (menu.active) {
            menu.draw( projectors[i].index );
        }

    }

}

/******************************************

 SETTINGS

 *******************************************/

void vdome::loadXML(ofXml &xml) {

    render.loadXML(xml);
    model.loadXML(xml);
    socket.loadXML(xml);

    winCount = 0;
    projCount = 0;



    if (xml.exists("window")) {
        winCount = xml.getNumChildren("window");
        for (int i=0; i<winCount; i++) {
            Window w;
            xml.setTo("window["+ ofToString(i) + "]");

            w.firstProjector = projCount;
            projCount += xml.getNumChildren();
            w.lastProjector = projCount-1;
            if (i > 0) glfw->createWindow();
            w.glfwWindow = glfwWindows->at(i);

            for (int j=0; j<projCount-w.firstProjector; j++) {
                Projector p;
                projectors.push_back(p);
            }

            windows.push_back(w);
            xml.setToParent();
        }
    }
    int c=0;
    ofPoint pos(0,0);
    for (int i=0; i<winCount; i++) {
        xml.setTo("window["+ ofToString(i) + "]");
        windows[i].loadXML(xml);
        pos.set(0, 0);

        for (int j=0; j<=windows[i].lastProjector-windows[i].firstProjector; j++) {
            xml.setTo("projector["+ ofToString(j) + "]");

            projectors[c].init(c);
            projectors[c].loadXML(xml);

            if (j != 0) {
                pos.x += projectors[c-1].width;
                pos.y += 0;
            }
            projectors[c].setPlanePosition(pos.x, pos.y);
            projectors[c].setup();
            projectors[c].loadXML2(xml);

            xml.setToParent();
            c++;
        }

        xml.setToParent();
    }

    if (xml.exists("[@autosave]")) {
        string str = ofToString( xml.getAttribute("[@autosave]") );
        if (str == "on")    autosave = true;
        else                autosave = false;
    }


    input.loadXML(xml);

}

void vdome::saveXML(ofXml &xml) {

    socket.saveXML(xml);
    input.saveXML(xml);
    render.saveXML(xml);
    model.saveXML(xml);

    int c = 0;
    for (int i=0; i<winCount; i++) {

        xml.setTo("window["+ ofToString(i) + "]");
        windows[i].saveXML(xml);

        for (int j=0; j<=windows[i].lastProjector-windows[i].firstProjector; j++) {
            xml.setTo("projector["+ ofToString(j) + "]");
            projectors[c].saveXML(xml);
            xml.setToParent();
            c++;
        }

        xml.setToParent();

    }
    saveThread.save();
}

/******************************************

 MOUSE

 ********************************************/

void vdome::mousePressed(ofMouseEventArgs& mouseArgs) {
    menu.mousePressed(mouseArgs);
}

void vdome::mouseDragged(ofMouseEventArgs& mouseArgs) {
    menu.mouseDragged(mouseArgs);

}

void vdome::mouseReleased(ofMouseEventArgs& mouseArgs) {
    menu.mouseReleased(mouseArgs);
    if (autosave && menu.active)
        saveXML(xml);
}

/******************************************

 KEYBOARD

 ********************************************/

void vdome::keyPressed(int key){
    menu.keyPressed(key);
	if (ofGetKeyPressed(cKey) && (ofGetKeyPressed(115) || ofGetKeyPressed(19)) && !autosave) { // ctrl + s = save
        saveXML(xml);
    }
}

void vdome::keyReleased(int key){
    menu.keyReleased(key);
    if (key != OF_KEY_UP && key != OF_KEY_DOWN) {
        if (autosave && menu.active)
            saveXML(xml);
    }
    #ifdef TARGET_OSX
    #else
	if (ofGetKeyPressed(cKey) && (key == 113 || key == 17)) // ctrl+q = quit
		ofExit(0);
    #endif
}

/******************************************

 FILE

 ********************************************/

void vdome::dragEvent(ofDragInfo dragInfo){
    input.dragEvent(dragInfo);
}

/******************************************

 EXIT

 ********************************************/

void vdome::exit(){
    saveThread.waitForThread(true);
    input.stop();
    input.close();
}

}
