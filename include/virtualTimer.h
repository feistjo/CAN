/**
 * @file virtualTimer.h
 * @author Chris Uustal
 * @brief Offers virtual timer functionality for task execution and checking if a certain duration of time has elapsed (in ms).
 * 			For general task execution purposes, make a new virtualTimerGroup_S object and addTimer(). For time elapsed, make
 * 			a new virtualTimer_S and start(), then getElapsedTime() or hasTimerExpired() for use in your code. You can combine 
 * 			your own virtual timers and those created using addTimer() in a group by calling addTimer() with a pointer to your
 *			virtual timer.
 * @version 1
 * @date 2022-09-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef VIRTUALTIMERS_H
#define VIRTUALTIMERS_H

#include <Arduino.h>

#define MAX_TIMERS 16

typedef enum
{
	TIMER_NOT_STARTED,
	TIMER_RUNNING,
	TIMER_EXPIRED,
}virtualTimer_state_E;

typedef struct virtualTimer_S
{
	unsigned long prevTick = 0U;
	unsigned long duration = 100U;

	virtualTimer_S* prevTimer = NULL;
	virtualTimer_S* nextTimer = NULL;

	void (*func)(void) = NULL;

	virtualTimer_state_E timerState = TIMER_NOT_STARTED;

	/**
	 * @brief Start a timer without a function using the given duration
	 * 
	 * @param durationInput = duration until timer expires
	 */
	void start (unsigned long durationInput)
	{
		duration = durationInput;
		timerState = TIMER_RUNNING;
		prevTick = millis();
	}

	/**
	 * @brief Get the current state of the timer
	 * 
	 * @return Current state of the timer
	 */
	virtualTimer_state_E getTimerState()
	{
		if (millis() >= prevTick + duration)
		{
			timerState = TIMER_EXPIRED;
		}

		return timerState;
	}

	/**
	 * @brief Check if the timer has expired and update timerState if so 
	 * 
	 * @return true = timer has exceeded duration 
	 * @return false = timer has not exceeded duration 
	 */
	bool hasTimerExpired()
	{
		bool ret = false;
		if (getTimerState() == TIMER_EXPIRED)
		{
			ret = true;
		}
		return ret;
	}

	/**
	 * @brief Get the time (in ms) that has elapsed since the timer was started
	 * 
	 * @return Elapsed time since timer was started
	 */
	unsigned long getElapsedTime()
	{
		unsigned long ret = millis() - prevTick;
		return ret;
	}

	/**
	 * @brief Initialize the duration, prevTick, and function pointers for this timer 
	 * 
	 * @param durationInput = duration between function calls 
	 * @param runFunc = function to be called when ticked
	 */
	void init (unsigned long durationInput, void (*runFunc)(void))
	{
		duration = durationInput;
		func = runFunc;
		timerState = TIMER_RUNNING;
		prevTick = millis();
	}

	/**
	 * @brief Run the fuction associated with this timer and update prevTick 
	 * 
	 * @return true if the function returned faster than the timer duration 
	 * @return false if the function took longer to execute than the timer duration 
	 * 			or the function pointer is NULL
	 */
	bool tick ()
	{
		bool ret = true;

		if (func != NULL)
		{
			if (millis() >= prevTick + duration)
			{
				prevTick = millis();
				timerState = TIMER_RUNNING;

				func();

				if (millis() >= prevTick + duration)
				{
					ret = false;
				}
			}
		}
		else 
		{
			ret = false;
		}
		
		return ret;
	}

}virtualTimer_S;

typedef struct 
{
	virtualTimer_S* headNode = NULL;
	virtualTimer_S* tailNode = NULL;
	unsigned long minTimerDuration;

	virtualTimer_S timerArr[MAX_TIMERS];
	int arrPos = 0;

	/**
	 * @brief Add an existing timer to the timer group 
	 * 
	 * @param newTimer = existig timer to be added to the group
	 */
	void addTimer(virtualTimer_S* newTimer)
	{
		if (headNode == NULL)
		{
			headNode = newTimer;
			tailNode = newTimer;
			newTimer->prevTimer = newTimer;
			newTimer->nextTimer = newTimer;
			minTimerDuration = newTimer->duration;
		}
		else
		{
			newTimer->prevTimer = tailNode;
			newTimer->nextTimer = headNode;

			headNode->prevTimer = newTimer;
			tailNode->nextTimer = newTimer;
			tailNode = newTimer;

			if (newTimer->duration < minTimerDuration)
			{
				minTimerDuration = newTimer->duration;
			}
		}
	}

	/**
	 * @brief Create a new timer with the given duration and function and add it to the group 
	 * 
	 * @param duration = duration between function calls 
	 * @param runFunc = function to be run when ticked 
	 * @return true = timer was added successfully 
	 * @return false = failed to add timer (likely reached the max number of timers)
	 */
	bool addTimer(unsigned long duration, void(*runFunc)(void))
	{
		bool ret = true;
		if(arrPos < MAX_TIMERS)
		{
			if (headNode == NULL)
			{
				timerArr[0].init(duration, runFunc);
				
				headNode = &timerArr[0];
				tailNode = &timerArr[0];
				timerArr[0].prevTimer = &timerArr[0];
				timerArr[0].nextTimer = &timerArr[0];
				minTimerDuration = duration;

				arrPos++;
			}
			else
			{
				timerArr[arrPos].init(duration, runFunc);
				
				timerArr[arrPos].prevTimer = tailNode;
				timerArr[arrPos].nextTimer = headNode;

				headNode->prevTimer = &timerArr[arrPos];
				tailNode->nextTimer = &timerArr[arrPos];
				tailNode = &timerArr[arrPos];

				if (duration < minTimerDuration)
				{
					minTimerDuration = duration;
				}

				arrPos++;
			}
		}
		else
		{
			ret = false;
		}
		return ret;
	}

	/**
	 * @brief Tick all timers in group
	 * 
	 * @return true if all tasks ran faster than minimum timer duration 
	 * @return false if one or more tasks took longer than the minimum timer duration 
	 * 			(timers may no longer fire at the correct interval)
	 */
	bool tick()
	{
		bool ret = true;

		if (headNode != NULL)
		{
			virtualTimer_S* tempTimer = headNode;
			unsigned long tempTaskDuration;
			do
			{
				tempTaskDuration = millis();
				tempTimer->tick();
				tempTaskDuration = millis() - tempTaskDuration;

				if (tempTaskDuration > minTimerDuration)
				{
					ret = false;
				}

				tempTimer = tempTimer->nextTimer;
			} while (tempTimer != headNode);
		}

		return ret;
	}

}virtualTimerGroup_S;

#endif