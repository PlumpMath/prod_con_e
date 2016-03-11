// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


//
// This benchmarks measures the latency of responding to messages in Theron.
// Latency refers to the delay, or elapsed time, between sending a message
// to an actor and receiving a response. Even if Theron is capabale of high throughput
// (handling millions of messages per second across a number of actors), the latency
// of handling individual messages is an independent concern and equally important for
// specialized applications where fast responses are important.
//
// The ping-pong benchmark is a standard microbenchmark commonly used to measure the
// message processing speed of concurrent systems such as computer networks.
// * Create two actors, called Ping and Pong.
// * Ping is set up to send any non-zero integer messages it receives to Pong, decremented by one.
// * Pong is set up to send any non-zero integer messages it receives to Ping, decremented by one.
// * On receipt of a zero integer message, Ping and Pong send a signal message to the client code indicating completion.
// * Processing is initiated by sending a non-zero integer message to Ping.
//
// The work done by the benchmark consists of sending n messages between Ping and Pong, where
// n is the initial value of the integer message initially sent to Ping. The latency of the
// message sending is calculated as the total execution time divided by the number of messages n.
//


#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <Theron/Theron.h>

#include "../Common/Timer.h"

class Producer : public Theron::Actor
{
public:

    struct StartMessage
    {
        inline StartMessage(const Theron::Address &caller, const Theron::Address &consumer, const Theron::Address &eventQ) :
          mCaller(caller),
          mConsumer(consumer),
          mEventQ(eventQ)
        {
        }

        Theron::Address mCaller;
        Theron::Address mConsumer;
        Theron::Address mEventQ;
    };

    inline Producer(Theron::Framework &framework) : Theron::Actor(framework)
    {
        RegisterHandler(this, &Producer::Start);
    }

private:

    inline void Start(const StartMessage &message, const Theron::Address /*from*/)
    {
        mCaller   = message.mCaller;
        mConsumer = message.mConsumer;
        mEventQ   = message.mEventQ;

        std::cout << "Producer Start" << std::endl;

        DeregisterHandler(this, &Producer::Start);
        RegisterHandler(this, &Producer::Receive);
    }

    inline void Receive(const int &message, const Theron::Address /*from*/)
    {
        if (message > 0)
        {
            std::cout<<"Producer will start producing -> "<<message<<std::endl;
            Send(message, mEventQ);

            // Wait for Acknowledge
            DeregisterHandler(this, &Producer::Receive);
            RegisterHandler(this, &Producer::WaitForAck);
        }
        else
        {
            std::cout<<"Failed producing message | Ensure message is proper: "<<message<<std::endl;
            Send(message, mCaller);
        }
    }

    inline void WaitForAck(const int &message, const Theron::Address /*from EQ*/){

        if (message < 0){
            std::cout<<"Producer received ACK: "<<message<<std::endl;
            Send(message, mCaller);

            // Start Receiving message to produce
            DeregisterHandler(this, &Producer::WaitForAck);
            RegisterHandler(this, &Producer::Receive);
        }
        else
        {
            std::cout<<"Failed ACK signal | Ensure ACK is proper: "<<message<<std::endl;
            Send(message, mCaller);
        }
    }

    Theron::Address mCaller;
    Theron::Address mConsumer;
    Theron::Address mEventQ;
};
