#include "Weirwood.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"

using namespace Weirwood;
using namespace ci;
using namespace ci::app;

#include "Command.h"

// We'll create a new Cinder Application by deriving from the BasicApp class
class Grove : public AppBasic {
	public:
		Grove(void);
		~Grove(void);

		// Cinder will always call this function whenever the user drags the mouse
		void mouseDrag( MouseEvent event );
		void keyDown( KeyEvent event );
		void fileDrop( FileDropEvent event );
	
		// Cinder calls this function 30 times per second by default
		void draw();
		void update();
		virtual void setup();
private:
	Font mFont;
	__time64_t mScriptModificationTime;
	std::string mScriptPath;
	double mScriptParsingTime;
	void loadScriptFile(const std::string scriptPath);
	__time64_t queryScriptModTime(const std::string& scriptPath);
	// This will maintain a list of points which we will draw line segments between
	std::list<Vec2f> mPoints;
	Sprout* mSproutPtr;
	Processor* mProcessorPtr;
};