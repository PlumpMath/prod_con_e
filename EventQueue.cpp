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
#include <queue>

#include <Theron/Theron.h>

#include "../Common/Timer.h"

class EventQueue : public Theron::Actor
{
public:

    struct StartMessage
    {
        inline StartMessage(const Theron::Address &caller, const Theron::Address &producer, const Theron::Address &consumer):
          mCaller(caller),
          mProducer(producer),
          mConsumer(consumer)
        {
        }

        Theron::Address mCaller;
        Theron::Address mProducer;
        Theron::Address mConsumer;
    };

    inline EventQueue(Theron::Framework &framework) : Theron::Actor(framework)
    {
        RegisterHandler(this, &EventQueue::Start);
    }

private:

    inline void Start(const StartMessage &message, const Theron::Address /*from*/)
    {
        mCaller   = message.mCaller;
        mConsumer = message.mConsumer;
        mProducer = message.mProducer;

        std::cout << "EventQueue Start" << std::endl;

        DeregisterHandler(this, &EventQueue::Start);
        RegisterHandler(this, &EventQueue::Receive);
    }

    inline void Receive(const int &message, const Theron::Address /*from*/)
    {
        // XXX The message can be a struct which holds the type{producer|consumer}
        if (message > 0)
        {
            std::cout<<"Message from Producer | will be queued: "<<message<<std::endl;

            // Push into Producer Queue
            producerQueue.push(message);

            // XXX REORDER If required | OR wait

            // Get the message and send it to Consumer
            const int consumerMessage = producerQueue.front();
            producerQueue.pop();
            Send(consumerMessage, mConsumer);
        }
        else {
            std::cout<<"Message from Consumer | will be queued: "<<message<<std::endl;
            // Push into Consumer Queue
            consumerQueue.push(message);

            // XXX REORDER If required | OR wait

            // Get the message and send it to Producer
            const int producerMessage = consumerQueue.front();
            consumerQueue.pop();
            Send(producerMessage, mProducer);
            }
    }

    Theron::Address mCaller;
    Theron::Address mConsumer;
    Theron::Address mProducer;

    std::queue<int> producerQueue;
    std::queue<int> consumerQueue;
};
