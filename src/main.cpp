#include "ofMain.h"
#include "ofApp.h"

#include "ofAppGLFWWindow.h"

#include "config.h"

//========================================================================
int main( ){
    ofGLFWWindowSettings settings;

    settings.setGLVersion(3, 2);
    settings.stencilBits = 0;
    settings.setSize(STANDARD_PROJECTOR_WINDOW_WIDTH, STANDARD_PROJECTOR_WINDOW_HEIGHT);
    settings.resizable = true;
    settings.decorated = true;

    // OFlua main visual-programming window
    shared_ptr<ofAppGLFWWindow> ofluaWindow = dynamic_pointer_cast<ofAppGLFWWindow>(ofCreateWindow(settings));
    shared_ptr<ofApp> ofluaApp(new ofApp);

    ofRunApp(ofluaWindow, ofluaApp);
    ofRunMainLoop();

    // done
    return EXIT_SUCCESS;

}
