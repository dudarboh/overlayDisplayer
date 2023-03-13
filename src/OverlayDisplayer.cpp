#include "OverlayDisplayer.hpp"
#include "EVENT/LCCollection.h"
#include "marlinutil/DDMarlinCED.h"
#include "marlin/VerbosityLevels.h"
#include "UTIL/LCRelationNavigator.h"
#include "EVENT/MCParticle.h"
#include "EVENT/Track.h"
#include "UTIL/ILDConf.h"
#include "DDRec/Vector3D.h"
using namespace EVENT;
using namespace UTIL;
using dd4hep::rec::Vector3D;
OverlayDisplayer aOverlayDisplayer;


// we need this to get ALL hits of the PFO track, not only the first semi-circle for PFO drawing
std::vector<EVENT::Track*> getSubTracks(EVENT::Track* track){
    std::vector<Track*> subTracks;
    // track itself contains VXD+FTD+SIT+TPC hits of the first curl.
    subTracks.push_back(track);

    int nSubTracks = track->getTracks().size();
    if (nSubTracks <= 1) return subTracks;

    int nTPCHits = track->getSubdetectorHitNumbers()[(ILDDetID::TPC)*2-1];
    int nSubTrack0Hits = track->getTracks()[0]->getTrackerHits().size();
    int nSubTrack1Hits = track->getTracks()[1]->getTrackerHits().size();

    //OPTIMIZE: this is not reliable, but I don't see any other way at the moment.
    //Read documentation in the header file for details.
    int startIdx;
    if( std::abs(nTPCHits - nSubTrack0Hits) <= 1  ) startIdx = 1;
    else if ( std::abs(nTPCHits - nSubTrack1Hits) <= 1 ) startIdx = 2;
    else{
        //FIXME: This happens very rarily (0.01%) for unknown reasons, so we just, skip adding subTracks...
        streamlog_out(WARNING)<<"Can't understand which subTrack is responsible for the first TPC hits! Skip adding subTracks."<<std::endl;
        return subTracks;
    }
    for(int j=startIdx; j < nSubTracks; ++j) subTracks.push_back( track->getTracks()[j] );
    return subTracks;
}


void drawTrackerHits(EVENT::LCEvent* event, std::string colName){
    //drawing style of the points
    int type = 0; // point
    int layer = 1; // doesn't matter
    int size = 4; // larger point
    unsigned long colorDefault = 0x347cf7;
    unsigned long colorOverlay = 0xf50000;
    auto* colNames = event->getCollectionNames();

    if (std::find(colNames->begin(), colNames->end(), colName) == colNames->end() ){
        streamlog_out(MESSAGE)<<" Oops, didn't find "<<colName<<" hits collection"<<std::endl;
        return;
    }
    LCCollection* hits = event->getCollection(colName);
    LCRelationNavigator nav = LCRelationNavigator( event->getCollection("AllHitsRelations") );

    for (int i=0; i<hits->getNumberOfElements(); ++i){
        auto hit = static_cast <TrackerHit*> ( hits->getElementAt(i) );
        unsigned long color = colorDefault;
        auto objects = nav.getRelatedToObjects(hit);
        auto weights = nav.getRelatedToWeights(hit);
        if ( weights.empty() ) continue;

        int max_i = std::max_element(weights.begin(), weights.end()) - weights.begin();
        auto* simHit = static_cast<SimTrackerHit*> (objects[max_i]);
        auto* mc = simHit->getMCParticle();

        if ( mc->isOverlay() ) color = colorOverlay;
        auto pos = hit->getPosition();
        ced_hit_ID(pos[0], pos[1], pos[2], type, layer, size, color, 0 ); // draw hit
    }
    return;
}


void drawCalorimeterHits(EVENT::LCEvent* event, std::string colName){
    //drawing style of the points
    int type = 0; // point
    int layer = 1; // doesn't matter
    int size = 4; // larger point
    unsigned long colorDefault = 0x347cf7;
    unsigned long colorOverlay = 0xf50000;
    auto* colNames = event->getCollectionNames();

    if (std::find(colNames->begin(), colNames->end(), colName) == colNames->end() ){
        streamlog_out(MESSAGE)<<" Oops, didn't find "<<colName<<" hits collection"<<std::endl;
        return;
    }
    LCCollection* hits = event->getCollection(colName);
    LCRelationNavigator nav = LCRelationNavigator( event->getCollection("AllHitsRelations") );

    for (int i=0; i<hits->getNumberOfElements(); ++i){
        auto hit = static_cast <CalorimeterHit*> ( hits->getElementAt(i) );
        unsigned long color = colorDefault;
        auto objects = nav.getRelatedToObjects(hit);
        auto weights = nav.getRelatedToWeights(hit);
        if ( weights.empty() ) continue;

        int max_i = std::max_element(weights.begin(), weights.end()) - weights.begin();
        auto* simHit = static_cast<SimCalorimeterHit*> (objects[max_i]);
        MCParticle* mc = nullptr;
        double maxEdep = 0.;

        for(int j=0; j < simHit->getNMCContributions(); j++){
            double edep = simHit->getEnergyCont(j);
            if ( edep <= maxEdep ) continue;
            maxEdep = edep;
            mc = simHit->getParticleCont(j);
        }
        if ( mc->isOverlay() ) color = colorOverlay;
        auto pos = hit->getPosition();
        ced_hit_ID(pos[0], pos[1], pos[2], type, layer, size, color, 0 ); // draw hit
    }
    return;
}



//Draw all hits which were reconstructed as PFOs
void drawEventPFOs(EVENT::LCEvent* event){ 
    //drawing style of the points
    int type = 0; // point
    int layer = 1; // doesn't matter
    int size = 4; // larger point
    unsigned long colorDefault = 0x347cf7;
    unsigned long colorOverlay = 0xf50000;

    LCCollection* pfos = event->getCollection("PandoraPFOs");
    LCRelationNavigator nav ( event->getCollection("AllHitsRelations") );

    for (int i=0; i<pfos->getNumberOfElements(); ++i){
        auto pfo = static_cast <ReconstructedParticle*> ( pfos->getElementAt(i) );

        auto tracks = pfo->getTracks();
        for(auto* track: tracks){
            auto subTracks = getSubTracks(track);

            for(auto* subTrack : subTracks){
                auto hits = subTrack->getTrackerHits();

                for(auto* hit : hits){
                    //get MC of the hit and check if it is an overlay, if yes, draw with different color
                    unsigned long color = colorDefault;
                    auto objects = nav.getRelatedToObjects(hit);
                    auto weights = nav.getRelatedToWeights(hit);
                    if ( weights.empty() ) continue;

                    int max_i = std::max_element(weights.begin(), weights.end()) - weights.begin();
                    auto* simHit = static_cast<SimTrackerHit*> (objects[max_i]);
                    auto* mc = simHit->getMCParticle();

                    if ( mc->isOverlay() ) color = colorOverlay;
                    auto pos = hit->getPosition();
                    ced_hit_ID(pos[0], pos[1], pos[2], type, layer, size, color, 0 ); // draw hit
                }

            }        
        }        
        auto clusters = pfo->getClusters();
        for(auto* cluster: clusters){
            auto hits = cluster->getCalorimeterHits();

            for (auto* hit: hits){
                //get MC of the hit and check if it is an overlay, if yes, draw with different color
                unsigned long color = colorDefault;
                auto objects = nav.getRelatedToObjects(hit);
                auto weights = nav.getRelatedToWeights(hit);
                if ( weights.empty() ) continue;

                int max_i = std::max_element(weights.begin(), weights.end()) - weights.begin();
                auto* simHit = static_cast<SimCalorimeterHit*> (objects[max_i]);
                MCParticle* mc = nullptr;
                double maxEdep = 0.;

                for(int j=0; j < simHit->getNMCContributions(); j++){
                    double edep = simHit->getEnergyCont(j);
                    if ( edep <= maxEdep ) continue;
                    maxEdep = edep;
                    mc = simHit->getParticleCont(j);
                }

                if ( mc->isOverlay() ) color = colorOverlay;
                auto pos = hit->getPosition();
                ced_hit_ID(pos[0], pos[1], pos[2], type, layer, size, color, 0 ); // draw hit
            }
        }

    }

}



//Draw all reconstructed hits from all possible collections
void drawEventAllHits(EVENT::LCEvent* event){
    std::vector<std::string> trackerHitColNames = {"VXDTrackerHits", "SITTrackerHits", "FTDPixelTrackerHits",
                                                   "FTDStripTrackerHits", "SETTrackerHits", "TPCTrackerHits"};
    for(auto name : trackerHitColNames)
        drawTrackerHits(event, name);

    std::vector<std::string> calorimeterHitColNames = {"EcalBarrelCollectionRec", "EcalEndcapsCollectionRec", "HcalBarrelCollectionRec", "EcalEndcapRingCollectionRec"
                                                       "HcalEndcapRingCollectionRec", "HcalEndcapsCollectionRec", "MUON", "LCAL", "LHCAL", "BCAL"};

    for(auto name : calorimeterHitColNames)
        drawCalorimeterHits(event, name);
}




OverlayDisplayer::OverlayDisplayer() : marlin::Processor("OverlayDisplayer"), EventDisplayer(this){
    _description = "This processor draws an All reconstructed hits in the event. Red hits - created by overlay. Blue - rest.";
}


void OverlayDisplayer::init(){
}

void OverlayDisplayer::processEvent(EVENT::LCEvent * event){
    ++_nEvent;
    streamlog_out(MESSAGE)<<std::endl<<"==========Event========== "<<_nEvent<<std::endl;
    // This draws hits from all sensible hit collections
    drawDisplay(this, event, drawEventAllHits, event);
    drawDisplay(this, event, drawEventPFOs, event);
}

void OverlayDisplayer::end(){
}