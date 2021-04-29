#pragma once

#include "ofMain.h"

#if !defined(TARGET_WIN32)
#include <pwd.h>
#endif

#include <unistd.h>

#include "ofxLua.h"

#include "config.h"

class ofApp : public ofBaseApp, ofxLuaListener {

public:
    void setup();
    void update();
    void draw();
    void exit();

    // Keyboard Events
    void keyPressed(int key);
    void keyReleased(ofKeyEventArgs &e);

    // Mouse Events
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseScrolled(int x, int y, float scrollX, float scrollY);

    // Misc Events
    void windowResized(int w, int h);

    // ofxLua error callback
    void errorReceived(std::string& msg);

    void            loadScript();

    void            initResolution();
    void            resetResolution(int newWidth, int newHeight);
    glm::vec2       reduceToAspectRatio(int _w, int _h);
    void            scaleTextureToWindow(int theScreenW, int theScreenH);
    void            toggleWindowFullscreen();
    void            resetOutputResolution();

    void            initDataFolderFromBundle();


    ofxLua                      lua;
    ofFile                      currentScriptFile;
    string                      filepath;
    bool                        scriptLoaded;
    bool                        isError;
    bool                        setupTrigger;
    bool                        loaded;

    ofFbo                       *fbo;
    ofImage                     *kuro;

    bool                        isFullscreen;
    int                         temp_width, temp_height;
    float                       output_width, output_height;
    int                         window_actual_width, window_actual_height;
    glm::vec2                   asRatio;
    glm::vec2                   window_asRatio;
    float                       thposX, thposY, thdrawW, thdrawH;
    bool                        needReset;

protected:

    size_t                      startTime;
    size_t                      waitTime;

    // BUNDLE
    std::filesystem::path       ofluaAppPath;
    string                      userHome;

    string                      tempstring;
    bool                        needToLoadScript;

};
