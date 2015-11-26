#include "socket.h"
namespace vd {

/******************************************

 CONSTRUCTOR

 ********************************************/

Socket::Socket(){
    enabled = true;
    host = "localhost";
    send = 3333;
    receive = 3334;
}

/******************************************

 SETUP

 ********************************************/

void Socket::setup(){
    oscSender.setup(host,send);
    oscReceiver.setup(receive);
}
    
/******************************************
 
 UPDATE
 
 ********************************************/

void Socket::update(){
    
    // receive
    while(oscReceiver.hasWaitingMessages()){
        rMsg.clear();
		oscReceiver.getNextMessage(&rMsg);
        
		if (rMsg.getAddress() == "/input/"){
            if      (rMsg.getArgAsString(0) == "play")         input->play();
            else if (rMsg.getArgAsString(0) == "stop")         input->stop();
            else if (rMsg.getArgAsString(0) == "previous")     input->previous();
            else if (rMsg.getArgAsString(0) == "next")         input->next();
		}
		else if (rMsg.getAddress() == "/input/loop/") {
            if (rMsg.getArgAsString(0) == "on") input->setLoop(true);
            else                                input->setLoop(false);
		}
        else if (rMsg.getAddress() == "/input/seek/") {
            input->seek(ofToFloat(rMsg.getArgAsString(0)));
		}
        else if (rMsg.getAddress() == "/input/source/") {
            input->setSource(rMsg.getArgAsString(0));
		}
        else if (rMsg.getAddress() == "/input/file/") {
            ofFile oFile;
            oFile.open(rMsg.getArgAsString(0));
            string file = oFile.getAbsolutePath();
            input->openFile(file);
		}
        else if (rMsg.getAddress() == "/input/volume/") {
            input->setVolume(ofToFloat(rMsg.getArgAsString(0)));
		}
        else if (rMsg.getAddress() == "/input/format/") {
            input->setFormat(rMsg.getArgAsString(0));
        }
	}
    
    // send
    sMsg.clear();
    if (input->isPlaying()) {
        sMsg.setAddress("/input/position");
        sMsg.addStringArg(ofToString( input->getPosition() ));
        oscSender.sendMessage(sMsg);
    }
}
    
    
/******************************************
 
 SEND
 
 ********************************************/

void Socket::sendDuration(){
    sMsg.clear();
    sMsg.setAddress("/input/duration");
    sMsg.addStringArg(input->getFilepath() + "," + ofToString( input->getDuration() ));
    oscSender.sendMessage(sMsg);
}
    
void Socket::sendEnd(){
    sMsg.clear();
    sMsg.setAddress("/input/");
    sMsg.addStringArg("end");
    oscSender.sendMessage(sMsg);
}
    
/******************************************

 SETTINGS

 ********************************************/

void Socket::loadXML(ofXml &xml) {
    if (xml.exists("socket[@enabled]")) {
        string str = ofToString(xml.getAttribute("socket[@enabled]"));
        if (str == "on") enabled = true;
        else             enabled = false;
    }
    
    if (xml.exists("socket[@host]"))
        host = ofToString( xml.getAttribute("socket[@host]") );
    if (xml.exists("socket[@send]"))
        send = ofToInt( xml.getAttribute("socket[@send]") );
    if (xml.exists("socket[@receive]"))
        receive = ofToInt( xml.getAttribute("socket[@receive]") );
    
    if (enabled)
        setup();
}

void Socket::saveXML(ofXml &xml) {
    xml.setTo("socket");

    if (enabled) xml.setAttribute("enabled", "on");
    else         xml.setAttribute("enabled", "off");
    
    xml.setAttribute("host", ofToString(host));
    xml.setAttribute("send", ofToString(send));
    xml.setAttribute("receive", ofToString(receive));

    xml.setToParent();
}

}
