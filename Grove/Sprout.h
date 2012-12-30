#pragma once
#include "Weirwood.h"
#include "cinder/Vector.h"

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
		};

		Sprout(ci::Vec2f origin, float unitLength);
		~Sprout(void);

		void ToState(State& out_state);
		void FromState(const State& state);

		void Reset();
		void Flush();
		void SetWidth(float width);

		void Move(float units);
		void SetPosition(float x, float y);

		void Rotate(float degree);
		void SetRotation(float degree);
	private:
		void Render(bool enabled);
		bool mRendering;
		float mStrokeWidth;
		float mUnitLength;
		ci::Vec2f mOrigin;
		ci::Vec2f mPosition;
		ci::Vec2f mDirection;
	};
}
