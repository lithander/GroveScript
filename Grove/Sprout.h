#pragma once
#include "Weirwood.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"

namespace Weirwood
{
	class Sprout
	{
	public:
		struct State
		{
			ci::Vec2f Position;
			ci::Vec2f Direction;
			float StrokeWidth;
			float Alpha;
			ci::Color Color;
		};

		Sprout(ci::Vec2f origin, float unitLength);
		~Sprout(void);
		void SetOrigin(ci::Vec2f origin);

		void ToState(State& out_state);
		void FromState(const State& state);

		void Reset();
		void Flush();
		void SetWidth(float width);

		void Move(float units);
		void Curve(float degree, float radius);

		void SetPosition(float x, float y);

		void Rotate(float degree);
		void SetRotation(float degree);
		void LookAt(float x, float y);

		void SetAlpha(float alpha);
		void SetColorRGB(float r, float g, float b);
		void SetColorHSV(float h, float s, float v);
	private:
		void Render(bool enabled);
		bool mRendering;
		float mStrokeWidth;
		float mUnitLength;
		float mAlpha;
		ci::Color mColor;
		ci::Vec2f mOrigin;
		ci::Vec2f mPosition;
		ci::Vec2f mDirection;
	};
}
