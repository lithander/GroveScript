#include "Weirwood.h"
#include "Sprout.h"
#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"

/*
bool mRendering;
float mStrokeWidth;
ci::Vec2f mPosition;
ci::Vec2f mDirection;
*/
using namespace cinder;
using namespace Weirwood;

Sprout::Sprout(Vec2f origin, float unitLength)
{
	mRendering = false;
	mOrigin = origin;
	mUnitLength = unitLength;
}

Sprout::~Sprout(void)
{
	Flush();
}

void Sprout::ToState(State& out_state)
{
	out_state.Position.set(mPosition);
	out_state.Direction.set(mDirection);
	out_state.StrokeWidth = mStrokeWidth;
}

void Sprout::FromState(const State& state)
{
	Flush();
	mPosition.set(state.Position);
	mDirection.set(state.Direction);
	mStrokeWidth = state.StrokeWidth;
}

void Sprout::Reset()
{
	Flush();
	mStrokeWidth = 1.0f;
	mPosition.set(mOrigin);
	mDirection.set(0.0, -mUnitLength);

	glLineWidth(mStrokeWidth);
	glColor3f( 1.0f, 0.5f, 0.25f );
}

void Sprout::Flush()
{
	Render(false);
}

void Sprout::SetWidth(float width)
{
	Render(false);
	glLineWidth(width);
}

void Sprout::Move(float units)
{
	Render(true);
	mPosition += units * mDirection;
	glVertex2f(mPosition);
}

void Sprout::SetPosition(float x, float y)
{
	Render(false);
	mPosition = mOrigin + Vec2f(x, -y);
}

void Sprout::Rotate(float degree)
{
	mDirection.rotate(toRadians(degree));
}

void Sprout::SetRotation(float degree)
{
	mDirection.set(0.0, -mUnitLength);
	mDirection.rotate(toRadians(degree));
}

//PRIVATE
void Sprout::Render(bool enabled)
{
	if(enabled && !mRendering)
	{
		glBegin(GL_LINE_STRIP);
		glVertex2f(mPosition);
		mRendering = true;
	}
	else if(!enabled && mRendering)
	{
		glEnd();
		mRendering = false;
	}
}

