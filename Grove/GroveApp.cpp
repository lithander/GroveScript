#include "Weirwood.h"
#include "GroveApp.h"
#include "ScriptReader.h"
#include "Sprout.h"
#include "Processor.h"

#include <sys/types.h>
#include <sys/stat.h>

using namespace Weirwood;

Grove::Grove(void) :
	mSproutPtr(NULL),
	mProcessorPtr(NULL),
	mSampleCount(0),
	mRunTimeSum(0),
	mStatsEnabled(false),
	mLogEnabled(true),
	mPostFxEnabled(true)
{
	//creating members in setup
}

Grove::~Grove(void)
{
	if(mProcessorPtr)
		delete mProcessorPtr;
	if(mSproutPtr)
		delete mSproutPtr;
}

void Grove::createFBOs(Vec2i size)
{
	gl::Fbo::Format format = gl::Fbo::Format();
	format.setSamples(8);
	mTargetFbo = gl::Fbo(size.x, size.y, format);
	mVerticalBlurFbo = gl::Fbo(size.x, size.y, format);
	mFinalBlurFbo = gl::Fbo(size.x, size.y, format);
}

void Grove::copyBlur(gl::Fbo& source, gl::Fbo& target, Vec2f sampleOffset)
{
	// bind the blur shader
	mBlurShader.bind();
	mBlurShader.uniform("tex0", 0); // use texture unit 0
	mBlurShader.uniform("sampleOffset", sampleOffset);

	// copy a horizontally blurred version of our scene into the first blur Fbo
	gl::setViewport( target.getBounds() );
	target.bindFramebuffer();
		source.bindTexture(0);
		gl::pushMatrices();
		gl::setMatricesWindow(target.getSize(), false);
			gl::clear( Color::black() );
			gl::drawSolidRect( target.getBounds() );
		gl::popMatrices();
		source.unbindTexture();
	target.unbindFramebuffer();	

	// unbind the shader
	mBlurShader.unbind();
}

void Grove::createShader()
{
	try
	{
		mBlurShader = gl::GlslProg(loadAsset("blur_vert.glsl"), loadAsset("blur_frag.glsl"));
		mCombineShader = gl::GlslProg(loadAsset("combine_vert.glsl"), loadAsset("combine_frag.glsl"));
	}
	catch(gl::GlslProgCompileExc ex)
	{
		console()<<"Failed to compile Shader"<<std::endl;
		console()<<ex.what()<<std::endl;
		quit();
	}
}

void Grove::setup()
{
	Vec2i size = getWindowSize();
	createFBOs(size);
	createShader();
	//AllocConsole();
	//freopen("CONOUT$", "wb", stdout); 
	mFont = Font("Lucida Console", 20);
	mSproutPtr = new Sprout(size / 2, 10.0f);
	mProcessorPtr = new Processor(mSproutPtr);
	if(getArgs().size() > 1)
		loadScriptFile(getArgs()[1]);
}

void Grove::keyDown( KeyEvent event )
{
	char input = event.getChar();
	if(input == 'f' )
		toggleFullscreen();
	else if(input == 's' )
		mStatsEnabled = !mStatsEnabled;
	else if(input == 'd' )
		mLogEnabled = !mLogEnabled;
	else if(input == 'b' )
		mPostFxEnabled = !mPostFxEnabled;
	else if(input == ' ' )
		createShader();
}

void Grove::toggleFullscreen()
{
	setFullScreen( !isFullScreen() );
	Vec2i size = getWindowSize();
	mSproutPtr->SetOrigin(size / 2);
	createFBOs(size);
}

void Grove::update() 
{
	//automatically reload on file change
	if(!mScriptPath.empty() && queryScriptModTime(mScriptPath) > mScriptModificationTime)
		loadScriptFile(mScriptPath);
}

void Grove::draw()
{
	if(!mScriptPath.empty())
	{
		if(mPostFxEnabled)
		{
			mTargetFbo.bindFramebuffer();
			gl::setMatricesWindow( mTargetFbo.getSize() );
		}
		else
			gl::setMatricesWindow( getWindowSize() );

		ci::Timer t;
		//run "Root" sequence
		gl::clear( Color::black() );
		if(mProcessorPtr->IsValid())
		{
			t.start();
			mProcessorPtr->ClearLog();
			mProcessorPtr->Run("Root");
			t.stop();
		}
		if(mPostFxEnabled)
			mTargetFbo.unbindFramebuffer();

		if(mPostFxEnabled)
		{
			//create a blurred copy of target
			copyBlur(mTargetFbo, mVerticalBlurFbo, Vec2f(1.0f/mVerticalBlurFbo.getWidth(), 0.0f));
			copyBlur(mVerticalBlurFbo, mFinalBlurFbo, Vec2f(0.0f, 1.0f/mVerticalBlurFbo.getHeight()));

			//clear screen
			gl::clear( Color::black() );
			gl::setMatricesWindow( getWindowSize() );

			//draw fbo+blur on window
			gl::pushModelView();
			gl::translate( Vec2f(0.0f, (float)getWindowSize().y) );
			gl::scale( Vec3f(1, -1, 1) );
			gl::color( Color::white() );
			mCombineShader.bind();
			mCombineShader.uniform("tex0", 0); // use texture unit 0
			mCombineShader.uniform("tex1", 1); // use texture unit 1
				mTargetFbo.bindTexture(0);
				mFinalBlurFbo.bindTexture(1);
					gl::drawSolidRect( getWindowBounds() );
				mTargetFbo.unbindTexture();
				mFinalBlurFbo.unbindTexture();
			mCombineShader.unbind();		
			gl::popModelView();
		}

		profileRunTime(t.getSeconds());
		Vec2f textOutPos(0,10);
		if(mStatsEnabled)
			printStats(textOutPos);
		
		//print accumulated LogMessages
		gl::enableAlphaBlending();
		std::string lastToken = "";
		for(Processor::LogMessageList::iterator it = mProcessorPtr->LogMessages().begin(); it != mProcessorPtr->LogMessages().end(); it++)
		{
			const std::string& msg = *it;
			int posError = msg.find("ERROR:");
			int posPrint = msg.find("PRINT:");
			if(posError != msg.npos)
			{
				gl::drawString(lastToken, textOutPos, ColorA::gray(0.8f), mFont);
				textOutPos.y += mFont.getSize();
				gl::drawString(msg.substr(6), textOutPos, ColorA::hex(0xFF0000), mFont);
				textOutPos.y += mFont.getSize();
			}
			else if(posPrint != msg.npos && mLogEnabled)
			{
				gl::drawString(msg.substr(6), textOutPos, ColorA::hex(0x00FFFF), mFont);
				textOutPos.y += mFont.getSize();
			}
			else
				lastToken = msg;			
		}
	}
	else
	{
		// clear out the window with black
		gl::clear( Color::gray( 0.5f ) );
		gl::enableAlphaBlending();
		gl::drawStringCentered( "Drag & Drop a Script file", getWindowCenter(), ColorA::white(), mFont);
	}
}

void Grove::fileDrop( FileDropEvent evt )
{
	fs::path path= evt.getFile( 0 );
	loadScriptFile(path.string());
}

void Grove::profileRunTime(double dt)
{
	mLastRunTime = dt;
	mSampleCount++;
	mRunTimeSum += mLastRunTime;
	double timeframe = 0.1;
	int runsPerTimeframe = (int)(timeframe / mLastRunTime);
	if(mSampleCount > runsPerTimeframe)
	{
		int prune = mSampleCount - runsPerTimeframe;
		double avg = mRunTimeSum / mSampleCount;
		mRunTimeSum -= prune * avg;
		mSampleCount -= prune;
	}
}

void Grove::printStats(Vec2f& textOutPos)
{
	std::stringstream ss;
	ss << mScriptPath << " Load Time: " << mScriptParsingTime << "s";
	gl::drawString(ss.str(), textOutPos, ColorA::white(), mFont);
	textOutPos.y += mFont.getSize();
	ss.str("");
	ss.setf(std::ios::fixed);
    ss.precision(6);
	ss << "FPS: " << (int)getFrameRate();
	gl::drawString(ss.str(), textOutPos, ColorA::hex(0xFFAAAA), mFont);
	textOutPos.y += mFont.getSize();
	ss.str("");

	ss << "Execution Time:  " << mLastRunTime << "s " << int(1.0 / mLastRunTime) << "/s";
	gl::drawString(ss.str(), textOutPos, ColorA::hex(0xFFAAAA), mFont);
	textOutPos.y += mFont.getSize();
	ss.str("");

	ss << "Avg. Exec. Time: " << mRunTimeSum / mSampleCount << "s " << int(mSampleCount / mRunTimeSum) << "/s";
	gl::drawString(ss.str(), textOutPos, ColorA::hex(0xFFAAAA), mFont);
	textOutPos.y += mFont.getSize();
	ss.str("");
}

__time64_t Grove::queryScriptModTime(const std::string& scriptPath)
{
	struct __stat64 fileinfo;
	if(-1 != _stat64(scriptPath.c_str(), &fileinfo))
		return fileinfo.st_mtime;
	else
		return 0; //exception?
}	

void Grove::loadScriptFile(const std::string scriptPath)
{
	mScriptPath = scriptPath;

	std::ifstream stream;
	stream.open(mScriptPath, std::fstream::in);
	if(!stream.good())
	{	
		console()<<"Failed to read file \""<<mScriptPath<<"\""<<std::endl;
		quit();
	}
	mScriptModificationTime = queryScriptModTime(mScriptPath);
	mProcessorPtr->Reset();
	ScriptReader reader;
	ci::Timer ti;
	ti.start();
	reader.Read(stream, mProcessorPtr);
	ti.stop();
	mScriptParsingTime = ti.getSeconds();
	stream.close();
}

// This line tells Flint to actually create the application
CINDER_APP_BASIC( Grove, RendererGl )
