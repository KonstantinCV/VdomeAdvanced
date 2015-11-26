#include "ofMain.h"
#include "vdome.h"
#include "ofxMultiGLFWWindow.h"
#include "ofGLProgrammableRenderer.cpp"

int main( ){

	#ifdef VDOME_DEBUG
	    ofSetLogLevel(OF_LOG_VERBOSE);
	#else
		ofSetLogLevel(OF_LOG_SILENT);
	#endif
	    string externalProjectorCount;

	ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);

	ofxMultiGLFWWindow prewin;
	prewin.setOpenGLVersion(3,2);
    prewin.windowCount = 1;
	prewin.hideBorder();
	ofSetupOpenGL(&prewin, 320, 240, OF_WINDOW);
	string empt = "";
	externalProjectorCount = "8";
 	while (externalProjectorCount!=empt){


    ofxMultiGLFWWindow win;
    win.setOpenGLVersion(3,2);
    win.windowCount = 1;
	win.hideBorder();

    ofSetupOpenGL(&win, 800, 600, OF_WINDOW);

	ofRunApp(new vd::vdome());
	   }

}
