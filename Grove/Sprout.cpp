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
	out_state.Color = mColor;
	out_state.Alpha = mAlpha;
}

void Sprout::FromState(const State& state)
{
	Flush();
	mPosition.set(state.Position);
	mDirection.set(state.Direction);
	mStrokeWidth = state.StrokeWidth;
	mColor = state.Color;
	mAlpha = state.Alpha;
}

void Sprout::Reset()
{
	Flush();
	mAlpha = 1.0f;
	mColor = ci::Color(1,1,1);
	mStrokeWidth = 1.0f;
	mPosition.set(mOrigin);
	mDirection.set(0.0, -mUnitLength);

	glLineWidth(mStrokeWidth);
	glColor4f(mColor.r, mColor.g, mColor.b, mAlpha);
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

void Sprout::SetOrigin(Vec2f origin)
{
	Render(false);
	mOrigin = origin;
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

void Sprout::LookAt(float x, float y)
{
	Vec2f target(x, -y);
	mDirection = (mOrigin + target) - mPosition;
	mDirection.normalize();
	mDirection *= mUnitLength;
}

void Sprout::SetColorRGB(float r, float g, float b)
{
	mColor = ci::Color(r,g,b);
	glColor4f(mColor.r, mColor.g, mColor.b, mAlpha);
}

void Sprout::SetColorHSV(float h, float s, float v)
{
	mColor = ci::hsvToRGB(ci::Vec3d(h,s,v));
	glColor4f(mColor.r, mColor.g, mColor.b, mAlpha);
}

void Sprout::SetAlpha(float alpha)
{
	mAlpha = alpha;
	glColor4f(mColor.r, mColor.g, mColor.b, mAlpha);
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

