#include "Weirwood.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"

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
		void keyDown( KeyEvent event );
		void fileDrop( FileDropEvent event );
	
		// Cinder calls this function 30 times per second by default
		void draw();
		void update();
		virtual void setup();
	private:
		Font mFont;
		std::string mScriptPath;

		bool mPostFxEnabled;
		gl::Fbo mTargetFbo;
		gl::Fbo mVerticalBlurFbo;
		gl::Fbo mFinalBlurFbo;
		gl::GlslProg mBlurShader;
		gl::GlslProg mCombineShader;
		
		//profiling
		bool mStatsEnabled;
		bool mLogEnabled;
		__time64_t mScriptModificationTime;
		double mScriptParsingTime;
		double mLastRunTime;
		int mSampleCount;
		double mRunTimeSum;

		void createFBOs(Vec2i size);
		void createShader();
		void copyBlur(gl::Fbo& source, gl::Fbo& target, Vec2f sampleOffset);
		void profileRunTime(double dt);
		void printStats(Vec2f& textOutPos);
		void toggleFullscreen();
		void setCanvasSize(Vec2i size);
		void loadScriptFile(const std::string scriptPath);
		__time64_t queryScriptModTime(const std::string& scriptPath);

		Sprout* mSproutPtr;
		Processor* mProcessorPtr;
};