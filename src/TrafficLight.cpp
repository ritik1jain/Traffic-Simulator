#include <iostream>
#include <random>
#include <mutex>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // perform queue modification under the lock
    std::unique_lock<std::mutex> lck(_mutex);
    while(_queue.empty()) {
        _condition.wait(lck);
    }

    // remove last element from queue
    T message = std::move(_queue.back());
    _queue.clear(); // calling clear method instead of pop_back method
                    // to prevent queue from stacking up at certain intersections
                    // This was causing the vehicles to go through the red light
                    // at those intersections. Using clear method fixes this issue

    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // perform queue modification under the lock
    std::lock_guard<std::mutex> lck(_mutex);

    // add message to queue
    _queue.push_back(std::move(msg));

    _condition.notify_one(); // notify client after pushing new message into queue
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}


void TrafficLight::waitForGreen()
{
    while (true)
    {
        TrafficLightPhase phase = _phaseQueue.receive();
        if (phase == TrafficLightPhase::green) { break; }
    }
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{   
    std::lock_guard<std::mutex> lck(_mtx);
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // launch cycleThroughPhases in a thread
    _threads.emplace_back(&TrafficLight::cycleThroughPhases_, this);
}

void TrafficLight::cycleThroughPhases_()
{
    // choose cycle duration randomly from between 4 and 6 seconds
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, 1);
    double cycleDuration = distr(eng) ? 4.0 : 6.0; // duration of a single simulation cycle in seconds

    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    // init stop watch
    lastUpdate = std::chrono::system_clock::now();
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
            // toggle traffic light phase
            TrafficLightPhase phase = getCurrentPhase();
            if (phase == TrafficLightPhase::green) { phase = TrafficLightPhase::red; }
            else { phase = TrafficLightPhase::green; }
            
            // set current phase
            std::unique_lock<std::mutex> lck(_mtx);
            _currentPhase = phase;
            lck.unlock();

            // update phaseQueue
            _phaseQueue.send(std::move(phase));

            // reset stop watch for next cycle
            lastUpdate = std::chrono::system_clock::now();
        }
        
    }  // eof simulation loop
}
