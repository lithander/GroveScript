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
	mProcessorPtr(NULL)
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

void Grove::setup()
{
	//AllocConsole();
	//freopen("CONOUT$", "wb", stdout); 
	mFont = Font("Lucida Console", 20);

	mSproutPtr = new Sprout(getWindowSize() / 2, 10.0f);
	mProcessorPtr = new Processor(mSproutPtr);
	if(getArgs().size() > 1)
		loadScriptFile(getArgs()[1]);
}

void Grove::mouseDrag( MouseEvent event )
{
	// add wherever the user drags to the end of our list of points
	mPoints.push_back( event.getPos() );
}

void Grove::keyDown( KeyEvent event )
{
	char input = event.getChar();
	if( input == 'f' )
		setFullScreen( ! isFullScreen() );
	if( input == ' ' )
		loadScriptFile(mScriptPath);
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
		gl::clear( Color::black() );
		//PROCESS COMMANDS
		gl::setMatricesWindow( getWindowSize() );
		
		ci::Timer t;
		if(mProcessorPtr->IsValid())
		{
			t.start();
			mProcessorPtr->ClearLog();
			mProcessorPtr->Run("Root");
			t.stop();
		}
		
		//PRINT COMMANDS
		gl::enableAlphaBlending();
		Vec2f textOutPos(0,10);
		gl::drawString(mScriptPath, textOutPos, ColorA::white(), mFont);
		textOutPos.y += mFont.getSize();
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
			else if(posPrint != msg.npos)
			{
				gl::drawString(msg.substr(6), textOutPos, ColorA::hex(0x00FFFF), mFont);
				textOutPos.y += mFont.getSize();
			}
			else
				lastToken = msg;			
		}
		std::stringstream ss;
		ss << "Execution Time: " << t.getSeconds() << "s";
		gl::drawString(ss.str(), textOutPos, ColorA::hex(0xFFAAAA), mFont);
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
	reader.Parse(stream, mProcessorPtr);
	stream.close();
}

// This line tells Flint to actually create the application
CINDER_APP_BASIC( Grove, RendererGl )
