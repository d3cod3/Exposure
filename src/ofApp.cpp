#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ///////////////////////////////////////////
    // OF Stuff
    ofSetEscapeQuitsApp(false);
    ofEnableAntiAliasing();

    initDataFolderFromBundle();
    ///////////////////////////////////////////

    // SYSTEM
    startTime   = ofGetElapsedTimeMillis();
    waitTime    = 200;

    isFullscreen            = false;
    thposX = thposY = thdrawW = thdrawH = 0.0f;

    output_width            = STANDARD_TEXTURE_WIDTH;
    output_height           = STANDARD_TEXTURE_HEIGHT;

    temp_width              = output_width;
    temp_height             = output_height;

    window_actual_width     = STANDARD_PROJECTOR_WINDOW_WIDTH;
    window_actual_height    = STANDARD_PROJECTOR_WINDOW_HEIGHT;

    scriptLoaded            = false;
    isError                 = false;
    setupTrigger            = false;

    ofSetWindowShape(window_actual_width, window_actual_height);

    // setup drawing  dimensions
    asRatio = reduceToAspectRatio(output_width,output_height);
    window_asRatio = reduceToAspectRatio(ofGetWindowWidth(),ofGetWindowHeight());
    scaleTextureToWindow(ofGetWindowWidth(),ofGetWindowHeight());

    initResolution();

    // load kuro
    kuro = new ofImage();
    kuro->load("images/kuro.jpg");

    // init lua
    lua.init(true);
    lua.addListener(this);

    // laod template script
    ofFile startScript(ofToDataPath("scripts/mainScript.lua",true));
    filepath = startScript.getAbsolutePath();

    // loaded flag
    loaded              = false;
}

//--------------------------------------------------------------
void ofApp::update(){
    ///////////////////////////////////////////
    // LUA UPDATE
    if(scriptLoaded && !isError){
        if(!setupTrigger){
            setupTrigger = true;
            lua.scriptSetup();
        }

        // update lua state
        ofSoundUpdate();
        lua.scriptUpdate();
    }
    ///////////////////////////////////////////

    ///////////////////////////////////////////
    // LUA DRAW
    fbo->begin();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    ofPushView();
    ofPushStyle();
    ofPushMatrix();
    if(scriptLoaded && !isError){
        lua.scriptDraw();
    }else{
        kuro->draw(0,0,fbo->getWidth(),fbo->getHeight());
    }
    ofPopMatrix();
    ofPopStyle();
    ofPopView();
    glPopAttrib();
    fbo->end();
    ///////////////////////////////////////////

    if(!loaded && ofGetElapsedTimeMillis()-startTime > waitTime){
        loaded = true;

        // setup drawing  dimensions
        asRatio = reduceToAspectRatio(output_width,output_height);
        window_asRatio = reduceToAspectRatio(ofGetWindowWidth(),ofGetWindowHeight());
        scaleTextureToWindow(ofGetWindowWidth(),ofGetWindowHeight());
        toggleWindowFullscreen();
        loadScript();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);

    ofSetColor(255);
    fbo->draw(thposX, thposY, thdrawW, thdrawH);
}

//--------------------------------------------------------------
void ofApp::exit() {
    ///////////////////////////////////////////
    // LUA EXIT
    lua.scriptExit();
    // clear the lua state
    lua.clear();
    ///////////////////////////////////////////
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    lua.scriptKeyPressed(key);
}

//--------------------------------------------------------------
void ofApp::keyReleased(ofKeyEventArgs &e){
    lua.scriptKeyReleased(e.key);
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    lua.scriptMouseMoved(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    lua.scriptMouseDragged(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    lua.scriptMousePressed(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    lua.scriptMouseReleased(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){
    lua.scriptMouseScrolled(x,y,scrollX,scrollY);
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    if(loaded){
        window_asRatio = reduceToAspectRatio(ofGetWindowWidth(),ofGetWindowHeight());
        scaleTextureToWindow(ofGetWindowWidth(),ofGetWindowHeight());
    }
}

//--------------------------------------------------------------
void ofApp::errorReceived(std::string& msg) {
    isError = true;

    if(!msg.empty()){
        size_t found = msg.find_first_of("\n");
        if(found == string::npos){
            ofLog(OF_LOG_ERROR,"LUA SCRIPT error: %s",msg.c_str());
        }
    }
}

//--------------------------------------------------------------
void ofApp::loadScript(){

    currentScriptFile.open(filepath);

    lua.doScript(filepath, true);

    tempstring = "OUTPUT_WIDTH = "+ofToString(output_width);
    lua.doString(tempstring);
    tempstring = "OUTPUT_HEIGHT = "+ofToString(output_height);
    lua.doString(tempstring);
    ofFile tempFileScript(filepath);
    tempstring = "SCRIPT_PATH = '"+tempFileScript.getEnclosingDirectory().substr(0,tempFileScript.getEnclosingDirectory().size()-1)+"'";

#ifdef TARGET_WIN32
    std::replace(tempstring.begin(),tempstring.end(),'\\','/');
#endif

    lua.doString(tempstring);

    scriptLoaded = lua.isValid();

    ///////////////////////////////////////////
    // LUA SETUP
    if(scriptLoaded  && !isError){
        ofLog(OF_LOG_NOTICE,"[verbose] lua script: %s loaded & running!",filepath.c_str());
    }
    ///////////////////////////////////////////
}

//--------------------------------------------------------------
void ofApp::initResolution(){
    fbo = new ofFbo();
    fbo->allocate(output_width,output_height,GL_RGBA32F_ARB,4);
    fbo->begin();
    ofClear(0,0,0,255);
    fbo->end();
}

//--------------------------------------------------------------
void ofApp::resetResolution(int newWidth, int newHeight){
    if(output_width!=newWidth || output_height!=newHeight){
        output_width    = newWidth;
        output_height   = newHeight;

        initResolution();

        tempstring = "OUTPUT_WIDTH = "+ofToString(output_width);
        lua.doString(tempstring);
        tempstring = "OUTPUT_HEIGHT = "+ofToString(output_height);
        lua.doString(tempstring);
        ofFile tempFileScript(filepath);
        tempstring = "SCRIPT_PATH = '"+tempFileScript.getEnclosingDirectory().substr(0,tempFileScript.getEnclosingDirectory().size()-1)+"'";

#ifdef TARGET_WIN32
        std::replace(tempstring.begin(),tempstring.end(),'\\','/');
#endif

        lua.doString(tempstring);
    }
}

//--------------------------------------------------------------
glm::vec2 ofApp::reduceToAspectRatio(int _w, int _h){
    glm::vec2 _res;
    int temp = _w*_h;
    if(temp>0){
        for(int tt = temp; tt>1; tt--){
            if((_w%tt==0) && (_h%tt==0)){
                _w/=tt;
                _h/=tt;
            }
        }
    }else if (temp<0){
        for (int tt = temp; tt<-1; tt++){
            if ((_w%tt==0) && (_h%tt==0)){
                _w/=tt;
                _h/=tt;
            }
        }
    }
    _res = glm::vec2(_w,_h);
    return _res;
}

//--------------------------------------------------------------
void ofApp::scaleTextureToWindow(int theScreenW, int theScreenH){
    // wider texture than screen
    if(asRatio.x/asRatio.y >= window_asRatio.x/window_asRatio.y){
        thdrawW           = theScreenW;
        thdrawH           = (output_height*theScreenW) / output_width;
        thposX            = 0;
        thposY            = (theScreenH-thdrawH)/2.0f;
        // wider screen than texture
    }else{
        thdrawW           = (output_width*theScreenH) / output_height;
        thdrawH           = theScreenH;
        thposX            = (theScreenW-thdrawW)/2.0f;
        thposY            = 0;
    }
    //ofLog(OF_LOG_NOTICE,"Window: %ix%i, Texture; %fx%f at %f,%f",theScreenW,theScreenH,thdrawW,thdrawH,thposX,thposY);
}

//--------------------------------------------------------------
void ofApp::toggleWindowFullscreen(){
    isFullscreen = !isFullscreen;
    ofToggleFullscreen();

    if(!isFullscreen){
        ofSetWindowShape(window_actual_width, window_actual_height);
        scaleTextureToWindow(window_actual_width, window_actual_height);
    }else{
        scaleTextureToWindow(ofGetScreenWidth(),ofGetScreenHeight());
    }
}

//--------------------------------------------------------------
void ofApp::resetOutputResolution(){

    if(output_width != temp_width || output_height != temp_height){
        output_width = temp_width;
        output_height = temp_height;

        resetResolution(output_width,output_height);

        asRatio = reduceToAspectRatio(output_width,output_height);

        if(!isFullscreen){
            window_asRatio = reduceToAspectRatio(ofGetWindowWidth(),ofGetWindowHeight());
            scaleTextureToWindow(ofGetWindowWidth(),ofGetWindowHeight());
        }else{
            window_asRatio = reduceToAspectRatio(ofGetScreenWidth(),ofGetScreenHeight());
            scaleTextureToWindow(ofGetScreenWidth(),ofGetScreenHeight());
        }


        ofLog(OF_LOG_NOTICE,"RESOLUTION CHANGED TO %ix%i",static_cast<int>(output_width),static_cast<int>(output_height));
    }

}

//--------------------------------------------------------------
void ofApp::initDataFolderFromBundle(){

#ifdef TARGET_OSX

    string _bundleDataPath;

    CFURLRef appUrl = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef appPath = CFURLCopyFileSystemPath(appUrl, kCFURLPOSIXPathStyle);

    const CFIndex kCStringSize = 128;
    char temporaryCString[kCStringSize];
    bzero(temporaryCString,kCStringSize);
    CFStringGetCString(appPath, temporaryCString, kCStringSize, kCFStringEncodingUTF8);
    std::string *appPathStr = new std::string(temporaryCString);
    CFRelease(appUrl);
    CFRelease(appPath);

    CFURLRef resourceUrl = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
    CFStringRef resourcePath = CFURLCopyFileSystemPath(resourceUrl, kCFURLPOSIXPathStyle);

    bzero(temporaryCString,kCStringSize);
    CFStringGetCString(resourcePath, temporaryCString, kCStringSize, kCFStringEncodingUTF8);
    std::string *resourcePathStr = new std::string(temporaryCString);
    CFRelease(resourcePath);
    CFRelease(resourceUrl);

    _bundleDataPath = *appPathStr + "/" + *resourcePathStr + "/"; // the absolute path to the resources folder

    const char *homeDir = getenv("HOME");

    if(!homeDir){
        struct passwd* pwd;
        pwd = getpwuid(getuid());
        if (pwd){
            homeDir = pwd->pw_dir;
        }
    }

    string _AppDataPath(homeDir);
    userHome = _AppDataPath;

    _AppDataPath += "/Documents/Exposure/data";

    std::filesystem::path tempPath(_AppDataPath.c_str());

    ofluaAppPath = tempPath;

    ofDirectory appDir;

    // data directory
    if(!appDir.doesDirectoryExist(ofluaAppPath)){
        appDir.createDirectory(ofluaAppPath,true,true);

        std::filesystem::path dataPath(_bundleDataPath.c_str());

        ofDirectory dataDir(dataPath);
        dataDir.copyTo(ofluaAppPath,true,true);
    }else{
        string relfilepath = _AppDataPath+"/release.txt";
        std::filesystem::path releasePath(relfilepath.c_str());
        ofFile relFile(releasePath);

        if(relFile.exists()){
            string actualRel = relFile.readToBuffer().getText();

            if(VERSION != actualRel){
                std::filesystem::path dataPath(_bundleDataPath.c_str());

                ofDirectory dataDir(dataPath);
                dataDir.copyTo(ofluaAppPath,true,true);
            }
        }
    }

    ofSetDataPathRoot(ofluaAppPath); // tell OF to look for resources here

#else



#endif

}
