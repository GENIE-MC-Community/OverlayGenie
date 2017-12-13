#ifndef EVENT_SOURCE_H
#define EVENT_SOURCE_H

// 
// A class for managing a source of genie events
//

#include "TH1.h"
#include "TChain.h"
#include <vector>
#include <map>
#include <exception>
#include "EVGCore/EventRecord.h"
#include "Ntuple/NtpMCFormat.h"
#include "Ntuple/NtpMCTreeHeader.h"
#include "Ntuple/NtpMCEventRecord.h"
//#include "Messenger/Messenger.h"
//#include "PDG/PDGCodes.h"
//#include "Utils/CmdLnArgParser.h"
class TRandom;

typedef genie::NtpMCEventRecord event ; 
typedef std::vector<event*> events;
typedef std::map<int, int> reuse_table; // event_number, n_reuse

class all_done : public std::exception {
 public:
  all_done(const char* _why="I think we are done"){ why=_why;}
  const char * what () const throw () {
    return why;
  }
  const char* why;
};

class event_source {
 public:
  event_source(TChain* c,  int seed=31459, int reuse_max=1, bool random_skip=false);
  ~event_source();

  // passes back events, owned by the caller
  // throws exception all_done when the source is dry
  
  // generate a number of events from a poisson distribution
  events get_poisson(float mu);
  // poisson, but it's assured to be non-zero frac of the time
  // return an event weight too  
  events get_poisson_assure_nonzero(float mu,float& weight, float frac=1);
  // generate a fixed number of events
  events get_fixed(int n);

  //// functions to help distribute the events over time
  //// the caller can use these if it wants
  // distribute the events uniformly in time
  void distribute_in_time(events& v, Double_t start, Double_t end);
  // distribute the events by sampling from a TH1
  void distribute_in_time(events& v, TH1* time_profile);
  // a function to time order the events
  static void time_order(events& v);

  // a function to make a fake event to mark the end of a spill
  static event* make_spill_end_marker(int ipdg=85, float time_ns=20e3);
  


 private:
  //////////////////////////////////////////////////////////////////////
  // get a single event
  event* get_event();
  // in !random_skip mode this will be the next entry  
  // this function keeps the books on the reuse_table
  // it will throw an exception of type all_done
  // if it hits the end of the stream or needs to stop
  // becuase reuse_max has been hit;
  
  
  void change_event_time(event* evt, Double_t new_time);


  TChain* c;
  event* ev;
  int reuse_max; 
  bool random_start;
  bool random_skip;
  TRandom* random_generator;
  Long64_t min_entry_number;
  Long64_t max_entry_number;
  Long64_t current_entry;
  reuse_table rtable;
};

#endif // EVENT_SOURCE_H

  
