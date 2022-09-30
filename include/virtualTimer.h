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

#include <stdint.h>
#include <vector>

class VirtualTimer
{
	public:
		/********** ENUMS **********/
		enum class State
		{
			kNotStarted,
			kRunning,
			kExpired,
		};

		enum class Type
		{
			kSingleUse,
			kRepeating,
			kUninitialized,
		};

		/********** PROTOTYPES **********/
		VirtualTimer ();
		VirtualTimer (uint32_t duration_ms, void (*task_func)(void), Type timer_type);
		void init (uint32_t duration_ms, void (*task_func)(void), Type timer_type);
		void start (uint32_t current_time);
		State getTimerState ();
		bool hasTimerExpired (uint32_t current_time);
		uint32_t getElapsedTime (uint32_t current_time);
		bool tick (uint32_t current_time);

		/********** VARIABLES **********/
		uint32_t duration;

	private: 
		uint32_t prev_tick = 0U;

		void (*task_func)(void) = NULL;

		State state = State::kNotStarted;
		Type type;
};

class VirtualTimerGroup
{
	public:
		/********** PROTOTYPES **********/
		VirtualTimerGroup ();
		void addTimer (VirtualTimer* new_timer);
		void addTimer (uint32_t duration_ms, void(*task_func)(void));
		bool tick (uint32_t current_time);

	private:
		uint32_t prev_tick = 0U;
		uint32_t min_timer_duration;
		std::vector<VirtualTimer> timer_group;
};

#endif
