/**
 * @file virtualTimer.cpp
 * @author Chris Uustal
 * @brief Defines some of then longer functions declared in the virtualTimer.h file 
 * @version 1
 * @date 2022-09-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "virtualTimer.h"

/**
 * @brief Default constructor for VirtualTimer object 
 * 
 */
VirtualTimer::VirtualTimer()
{
    duration = 0U;
    task_func = NULL;
    type = Type::kUninitialized;
}

/**
 * @brief Construct a new Virtual Timer:: Virtual Timer object
 * 
 * @param duration_ms - Duration the timer should fire after
 * @param task_func - Function to be called when timer expires 
 * @param timer_type - If the timer should be repeating or single-use 
 */
VirtualTimer::VirtualTimer(uint32_t duration_ms, void (*task_func)(void), Type timer_type)
{
    if (duration_ms != 0U)
    {
        duration = duration_ms;
        this->task_func = task_func;
        type = timer_type;
    }
}

/**
 * @brief Constructor duplicate for times when we can't use a parametrized constructor 
 * 
 * @param duration_ms - Duration the timer should fire after
 * @param task_func - Function to be called when timer expires 
 * @param timer_type - If the timer should be repeating or single-use 
 */
void VirtualTimer::init(uint32_t duration_ms, void (*task_func)(void), Type timer_type)
{
    if (duration_ms != 0U)
    {
        duration = duration_ms;
        this->task_func = task_func;
        type = timer_type;
    }
}

/**
 * @brief Start the associated timer 
 * 
 * @param current_time - the current time in ms at the time of calling (eg. millis())
 */
void VirtualTimer::start(uint32_t current_time)
{
    state = State::kRunning;
    prev_tick = current_time;
}

/**
 * @brief Get the current state of the timer (do not update it)
 * 
 * @param current_time - the current time in ms at the time of calling (eg. millis())
 * @return Current state of the timer
 */
VirtualTimer::State VirtualTimer::getTimerState()
{
    return state;
}

/**
 * @brief Check if the timer has expired but do not update it 
 * 
 * @param current_time - the current time in ms at the time of calling (eg. millis())
 * @return true = timer has exceeded duration 
 * @return false = timer has not exceeded duration 
 */
bool VirtualTimer::hasTimerExpired(uint32_t current_time)
{
    bool ret = false;
    if (getTimerState() == State::kExpired)
    {
        ret = true;
    }
    return ret;
}

/**
 * @brief Get the time (in ms) that has elapsed since the timer was started
 * 
 * @param current_time - the current time in ms at the time of calling (eg. millis())
 * @return Elapsed time since timer was started 
 */
uint32_t VirtualTimer::getElapsedTime(uint32_t current_time)
{
    uint32_t ret = current_time - prev_tick;
    return ret;
}

/**
 * @brief Run the fuction associated with this timer and update prev_tick if it's a repeating timer 
 * 
 * @param current_time - the current time in ms at the time of calling (eg. millis())
 * @return true if the function returned faster than the timer duration 
 * @return false if the function pointer is null or task missed an iteration
 */
bool VirtualTimer::tick(uint32_t current_time)
{
    bool ret = true;

    // Only tick a timer if it's already running 
    if (state == State::kRunning)
    {
        // Check if timer has expired 
        if (current_time >= prev_tick + duration)
        {
            if (type == Type::kRepeating)
            {
                // Check if the timer missed a cycle 
                if (current_time > prev_tick + 2*duration)
                {
                    ret = false;
                }
                // Update the last tick to the current time
                prev_tick = current_time;
            }
            else
            {
                // Timer isn't repeating, so it expires
                state = State::kExpired;
            }

            if (task_func != NULL)
            {
                // Run the associated task function
                task_func();
            }
            else 
            {
                ret = false;
            }
        }
    }
    
    return ret;
}

/**
 * @brief Default constructor for VirtualTimerGroup
 * 
 */
VirtualTimerGroup::VirtualTimerGroup()
{

}

/**
 * @brief Add an existing timer to the timer group 
 * 
 * @param newTimer = existig timer to be added to the group
 */
void VirtualTimerGroup::addTimer(VirtualTimer* new_timer)
{
    if (timer_group.empty())
    {
        min_timer_duration = new_timer->duration;
    }
    else if (new_timer->duration < min_timer_duration)
    {
        min_timer_duration = new_timer->duration;
    }

    timer_group.push_back(*new_timer);
}

/**
 * @brief Create a new timer with the given duration and function and add it to the group 
 * 
 * @param duration = duration between function calls 
 * @param runFunc = function to be run when ticked 
 * @return true = timer was added successfully 
 * @return false = failed to add timer (likely reached the max number of timers)
 */
void VirtualTimerGroup::addTimer(uint32_t duration_ms, void(*task_func)(void))
{
    VirtualTimer* new_timer = new VirtualTimer;

    new_timer->init(duration_ms, task_func, VirtualTimer::Type::kRepeating);

    if (timer_group.empty())
    {
        min_timer_duration = new_timer->duration;
    }
    else if (new_timer->duration < min_timer_duration)
    {
        min_timer_duration = new_timer->duration;
    }

    timer_group.push_back(*new_timer);
}

/**
 * @brief Tick all timers in group
 * 
 * @return true if all tasks ran faster than minimum timer duration 
 * @return false if the task loop took longer than the minimum timer duration 
 * 			(timers may no longer fire at the correct interval)
 */
bool VirtualTimerGroup::tick(uint32_t current_time)
{
    bool ret = true;

    if (timer_group.empty() == false)
    {
        // If the time since the last tick is longer than the shortest task
        if ((current_time - prev_tick) > min_timer_duration)
        {
            ret = false;
        }

        std::vector<VirtualTimer>::iterator i; 
        for (i = timer_group.begin(); i != timer_group.end(); i++)
        {
            if (i->getTimerState() != VirtualTimer::State::kRunning)
            {
                i->start(current_time);
            }
            
            if (i->tick(current_time) == false)
            {
                ret = false;
            }
        }
    }

    prev_tick = current_time;

    return ret;
}
