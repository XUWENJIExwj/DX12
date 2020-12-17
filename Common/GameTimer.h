#pragma once

class GameTimer
{
public:
	GameTimer();

	float TotalTime()const; // in seconds
	float DeltaTime()const; // in seconds
	bool  GetFixed()const { return mFixed; } // FixedUpdate�p

	void Reset(); // Call before message loop.
	void Start(); // Call when unpaused.
	void Stop();  // Call when paused.
	void Tick();  // Call every frame.
	void FixedTick(int TargetFPS); // FixedUpdate�p

private:
	double mSecondsPerCount;
	double mDeltaTime;

	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mExecLastTime; // FixedUpdate�p
	__int64 mCurrTime;

	bool mStopped;
	bool mFixed; // FixedUpdate�p
};