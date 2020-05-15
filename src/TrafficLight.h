#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"


// define TrafficLightPhase type
enum class TrafficLightPhase
{
    red,
    green,
};

// implement a generic concurrent message queue
template <class T>
class MessageQueue
{
public:
    void send(T &&msg);
    T receive();

private:
    std::deque<T> _queue;
    std::condition_variable _condition;
    std::mutex _mutex;
};

class TrafficLight : public TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();

    // getters / setters
    TrafficLightPhase getCurrentPhase();
    
    // typical behaviour methods
    void waitForGreen();
    void simulate() override;

private:
    // typical behaviour methods
    void cycleThroughPhases_();

    MessageQueue<TrafficLightPhase> _phaseQueue;
    TrafficLightPhase _currentPhase;
    std::mutex _mtx;  // to synchronize _currentPhase access
};

#endif