#ifndef OverlayDisplayer_h
#define OverlayDisplayer_h 1

#include "EventDisplayer.hpp"
#include "marlin/Processor.h"

class OverlayDisplayer : public marlin::Processor, EventDisplayer{
    friend class EventDisplayer;
    public:
        OverlayDisplayer(const OverlayDisplayer&) = delete;
        OverlayDisplayer& operator=(const OverlayDisplayer&) = delete;

        marlin::Processor* newProcessor() { return new OverlayDisplayer; }

        OverlayDisplayer();
        void init();
        void processEvent(EVENT::LCEvent* event);
        void end();
    private:
        int _nEvent{};
};

#endif
