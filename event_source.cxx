//____________________________________________________________________________
/*!

\program event_source

\brief   A class for managing a source of genie ghep events

\author  Mike Kordosky, Wiliam and Mary.

\created Nov 6, 2017

*/
//____________________________________________________________________________

#include "event_source.h"
#include "TRandom.h"
#include "TRandom3.h"
#include "TMath.h"
#include "TFile.h"
#include <algorithm>
#include <iostream>
#include "GHEP/GHepParticle.h"
#include "GHEP/GHepStatus.h"
#include "EVGCore/EventRecord.h"
#include "Interaction/Interaction.h"

using std::cout;
using std::endl;

event_source::event_source(TChain* _c,  int _seed, int _reuse_max, bool _random_skip){
 
  //  if(!c) return;
  c=_c;
  
  random_generator = new TRandom3(_seed);
  reuse_max=_reuse_max;
  random_skip = _random_skip;

  ev = new genie::NtpMCEventRecord();
  c->SetBranchAddress("gmcrec", &ev);
 
  min_entry_number=0;
  max_entry_number=c->GetEntries();
  
}

event_source::~event_source(){
  delete random_generator;
}

event* event_source::get_event(){
  // first figure out which event to get
  Long64_t next_entry=0;
  if(!random_skip){
    next_entry=current_entry+1;
  }
  else{
    // try to find a good, next entry

    // use the random number generator to find the next entry
    Long64_t proposed_next_entry=random_generator->Integer(max_entry_number+1);
    // look to see if the entry is in reuse_table
    // if so, make sure it hasn't been reused too many times
    std::map<int,int>::iterator search=rtable.find(proposed_next_entry);
    int times_already_used=0;
    if(search!=rtable.end()){
      times_already_used=search->second;
    }
    if(times_already_used>=reuse_max){
      // it's time to quit
      throw all_done("we hit the reuse_max barrier");
    }
    next_entry=proposed_next_entry;
    rtable[next_entry]=times_already_used+1;
  }
  if(next_entry>max_entry_number){
    // at the end of the file
    throw all_done("we are at the end of the file");
  }
  // now get the entry and copy it to make a new one
  //  cout<<"Getting entry "<<next_entry<<" from "<<c->GetFile()->GetName()<<endl;
  c->GetEntry(next_entry);
  //  cout<<*ev<<endl;
  event* evout = new event();
  evout->Copy(*ev);
  return evout;
  
}

events event_source::get_fixed(int n){
  events evts;
  for(int i=0; i<n; i++){
    event* an_event = get_event();
    evts.push_back(an_event);
  }
  return evts;
}

events event_source::get_poisson(float mu){
  int n=random_generator->Poisson(mu);
  if(n>0){
    return get_fixed(n);
  }
  else{
    events empty_event_list;
    return empty_event_list;
  }
}

events event_source::get_poisson_assure_nonzero(float mu, float& weight, float frac){
  if(frac>1.0 or frac<=0.0) {
    // frac=0 is probably allowed, but it's degenerate with get_poisson
    // and I suppose the caller is making a mistake in this case
    throw std::invalid_argument("frac must be >0 and =<1");
  }
  if(mu<=0) {
    throw std::invalid_argument("mu must be >0");
  }

  if(random_generator->Rndm()>frac){
    // pull from a normal poisson
    weight=1.0;
    return get_poisson(mu);
  }
  else{
    // require non-zero number of events
    int n=0;
    while(1){
      n=random_generator->Poisson(mu);
      if(n!=0) break;
    }
    weight=1-TMath::Exp(-mu);
    return get_fixed(n);
  }
  
}

void event_source::distribute_in_time(events& evts, Double_t start, Double_t end){
  //  std::vector<float> times;
  for(uint i=0; i<evts.size();i++){
    Double_t event_time=random_generator->Uniform(start,end);
    change_event_time(evts[i],event_time);
  }
}

void event_source::distribute_in_time(events& evts, TH1* time_profile){

  // make sure that we use our own generator to generate the times
  TRandom* gRandom_save=gRandom;
  gRandom=random_generator;

  for(uint i=0; i<evts.size();i++){
    Double_t event_time=time_profile->GetRandom();
    change_event_time(evts[i],event_time);
  }
  gRandom=gRandom_save;

}

bool timecomp(event* evt1, event* evt2){ 
  Double_t t1 = evt1->event->Vertex()->T();
  Double_t t2 = evt2->event->Vertex()->T();
  return (t1<t2);
}

void event_source::time_order(events& evts){
  std::sort(evts.begin(),evts.end(),timecomp);  
}

void event_source::change_event_time(event* evt, Double_t new_time){
  TLorentzVector* vtx=evt->event->Vertex();
  evt->event->SetVertex(vtx->X(), vtx->Y(), vtx->Z(),new_time);
}

event* event_source::make_spill_end_marker(int ipdg, float time_ns){
  
  genie::EventRecord* er=new genie::EventRecord();
  genie::Interaction* inter=new genie::Interaction();
  er->AttachSummary(inter);

  genie::GHepStatus_t status=genie::kIStUndefined; //GHEP/GHepStatus.h
  int mother1=0; int mother2=0;
  int daughter1=0; int daughter2=0;
  TLorentzVector v; v[3]=time_ns;
  TLorentzVector p;
  //  genie::GHepParticle* part = new genie::GHepParticle(ipdg, status, mother1, mother2, daughter1, daughter2, p, v);
  genie::GHepParticle part(ipdg, status, mother1, mother2, daughter1, daughter2, p, v);

  er->AddParticle(part);

  event* ev = new event();
  ev->Fill(0,er);
  return ev;

}
