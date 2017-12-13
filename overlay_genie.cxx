//____________________________________________________________________________
/*!

\program overlay_genie

\brief   Read in several genie ghep ntuples and combine them to produce an ghep ntuple appropriate for simulating overlaid spills.  

\author  Mike Kordosky, Wiliam and Mary.

\created Nov 6, 2017

\note  Makefile from GENIE gtestEventLoop.cxx

*/
//____________________________________________________________________________

#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>

#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
//#include <TIterator.h>

// option parsing via
// http://optionparser.sourceforge.net/
#include "optionparser.h"

#include "event_source.h"

//#include "EVGCore/EventRecord.h"
//#include "GHEP/GHepParticle.h"
//#include "Ntuple/NtpMCFormat.h"
#include "Ntuple/NtpMCTreeHeader.h"
#include "Ntuple/NtpMCEventRecord.h"
//#include "Messenger/Messenger.h"
//#include "PDG/PDGCodes.h"
//#include "Utils/CmdLnArgParser.h"

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using namespace genie;

enum  optionIndex { UNKNOWN, HELP, SOURCE, OUTPUT, TIMERANGE, TIMEHIST, NSPILL};


const char* source_help=" --source, -s  \t specify a neutrino source and overlay options\n"
" Format:\n -s/full/path/to/file.root,tree_name,seed,rmax,rskip,overlay_meth,overlay_par(s)\n" 
"    * seed (int) is a seed to be passed to the RNG (TRandom3)\n"
"    * rmax (int) is the maximum # of times an event can be reused\n"
"    * rskip y or n specifies that the file is read randomly (y) or linearly (n)\n"
"    * overlay_method and overlay_par(s) are one of\n" 
"      - fixed,<nevents> (integer)\n"
"          choose a fixed number of events for each spill\n"
"      - poisson,<mu> (float)\n"
"          draw from a Poisson distribution with parameter <mu> events per spill\n"
"      - poisson_anz,<mu>,<frac> (float),(float)\n"
"          like poisson but <frac> of the time generate a non-zero result\n" 
"          distributed according to a Poisson with parameter <mu>\n"
"          the method doing this also returns a weight which it \n"
"          records in the GHEPRecord::Weight() for all events \n"
"          created by this poisson_anz source for a given spill";

const option::Descriptor usage[] =
{
 {UNKNOWN, 0,"" , ""    ,option::Arg::None, "USAGE:  overlay_genie [options]\n\n"
  "NOTE: short option flags must not have a space between the flag and the option\n"
  "NOTE: long option flags must have an = between the flag and the option\n"
  "      -xmyoption is OK as is --longx=myoption\n"
  "      -x myoption fails as does --longx myoption\n"
                                            "Options:" },
 {HELP,    0,"h" , "help",option::Arg::None, " -h --help  \tPrint usage and exit." },
 {SOURCE,    0,"s", "source",option::Arg::Optional, source_help },
 {OUTPUT,    0,"o", "output",option::Arg::Optional, " -o --output outputfile.root,ntuplename" },
 {TIMERANGE,    0,"R", "time_range",option::Arg::Optional, " -R --time_range (float),(float)" },
 {TIMEHIST,    0,"H", "time_hist",option::Arg::Optional, " -H --time_hist rootfile,TH1_name" },
 {NSPILL,    0,"n", "nspills",option::Arg::Optional, " -n --nspills (integer)\n      Note: we cannot guarantee nspills as one of the sources\n      may run out first. It is a target/maximum." },
 // {TIMEORDER,    0,"t", "timeorder",option::Arg::None, " -t --timeorder  \n       Order the output events by time." },
 {0,0,0,0,0,0}
};


std::vector<std::string> split_option_string(const std::string& s)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (!tokenStream.eof())
   {
     std::getline(tokenStream, token, ',');
     tokens.push_back(token);
   }
   return tokens;
}


//___________________________________________________________________
int main(int argc, char ** argv)
{
  argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
  option::Stats  stats(usage, argc, argv);
  option::Option options[stats.options_max], buffer[stats.buffer_max];
  option::Parser parse(usage, argc, argv, options, buffer);
  if (parse.error())
    return 1;
  if (options[HELP] || argc == 0) {
    option::printUsage(std::cout, usage);
    return 0;
  }
  
  ////////////////// deal with the number of spills ///////////////////////////
  int nspill=0;
  if(options[NSPILL].count()==0){
    cout<<"ERROR: No spills requested! Why continue?!"<<endl;   
    option::printUsage(std::cout, usage);
    return 1;          
  }
  else{
    string nspill_s(options[NSPILL].last()->arg);
    bool is_non_neg_int = (nspill_s.find_first_not_of( "0123456789" ) == string::npos);
    if(is_non_neg_int) {
      char *end;
      nspill=std::strtol(nspill_s.c_str(),&end,10);
    }
    if(!is_non_neg_int || nspill==0){
      cout<<"ERROR: You have requested non integer or zero spills: "<<nspill_s<<endl;   
      cout<<"Can't continue!"<<endl;
      option::printUsage(std::cout, usage);
      return 1;     
    }
  }

  ////////////////////// deal with a timerange //////////////////////
  float tstart=0;
  float tend=0;
  if(options[TIMERANGE].count()!=0){
    string timerange_s(options[TIMERANGE].last()->arg);
    vector<string> timerange_option = split_option_string(timerange_s);
    bool are_floats = (timerange_option[0].find_first_not_of( "-+.0123456789" ) == string::npos) &&  (timerange_option[1].find_first_not_of( "-+.0123456789" ) == string::npos) ;
    if(are_floats){
      char* dummy;
      tstart=std::strtof(timerange_option[0].c_str(),&dummy);
      tend=std::strtof(timerange_option[1].c_str(),&dummy);
    }
    if(!are_floats || (tstart>tend)){
      cout<<"ERROR: Your time range "<<timerange_s
	  <<" doesn't consist of floats or tstart>tend"<<endl;
      cout<<"Can't continue!"<<endl;
      option::printUsage(std::cout, usage);
      return 1;     
    }
  }

  ////////////// deal with a time profile histogram /////////////////
  TH1* time_profile=0;
  if(options[TIMEHIST].count()!=0){
    string timehist_s(options[TIMEHIST].last()->arg);
    vector<string> timehist_option = split_option_string(timehist_s);
    if(timehist_option.size()!=2){
      cout<<"ERROR: Your time profile histogram option "<<timehist_s
	  <<" must have two fields: file,histname"<<endl;
      cout<<"Can't continue!"<<endl;
      option::printUsage(std::cout, usage);
      return 1;
    }
    string histfile=timehist_option[0];
    string histname=timehist_option[1];
    TFile fin(histfile.c_str(),"READ");
    time_profile=(TH1*) fin.Get(histname.c_str());
    if(!time_profile){
      cout<<"ERROR: Could not read histogram "<<histname
	  <<" from "<<histfile<<endl;
      cout<<"Can't continue!"<<endl;
      return 1;
    }
    time_profile->SetDirectory(0);
  }

  //////////////// deal with the output file ////////////////////////
  string output_filename="overlay_genie.root";
  string output_ntuple="gtree";

  if(options[OUTPUT].count()!=0){
    string output_s(options[OUTPUT].last()->arg);
    vector<string> output_option = split_option_string(output_s);
    if(output_option.size()!=2){
      cout<<"ERROR: Your time output file option "<<output_s
	  <<" must have two fields: file,ntuple"<<endl;
      cout<<"Can't continue!"<<endl;
      option::printUsage(std::cout, usage);
      return 1;
    }
    output_filename=output_option[0];
    output_ntuple=output_option[1];
  }
  
  //////////////// print any unknown options ////////////////////////
  for (option::Option* opt = options[UNKNOWN]; opt; opt = opt->next())
    std::cout << "Unknown option: " << opt->name << "\n";
  //  for (int i = 0; i < parse.nonOptionsCount(); ++i)
  //   std::cout << "Non-option #" << i << ": " << parse.nonOption(i) << "\n";

  
  //////////////// Loop over all of the source options ///////////////////
  // create event_source objects for each
  vector<event_source*> event_sources;
  struct source_opts{
    source_opts(string _method, float _mu, int _n, float _frac){
      method=_method; mu=_mu; n=_n; frac=_frac;
    }
    string method;
    float mu;
    int n;
    float frac;
  };
  vector<source_opts> event_sources_opts;

  int cntr=0;
  for (option::Option* opt = options[SOURCE]; opt; opt = opt->next()){
    cntr++;
    string opt_arg(opt->arg);
    vector<string> source_option = split_option_string(opt_arg);

    // get the overlay option and make sure we know what to do
    // this will find a bunch of syntax errors and allows more testing of the input
    string method=source_option[5];
    if(method!="fixed" && method!="poisson" && method!="poisson_anz"){
      cout<<"================ ERROR ============== "<<endl;
      cout<<" Unknown overlay method: "<<method<<endl;
      option::printUsage(std::cout, usage);
      return 1;
    }
    // at this point, we know the method is OK

    // check the number of arguments.
    if( ( ( (method=="fixed") || (method=="poisson") ) && (source_option.size()!=7) ) ||
	( ( method=="poisson_anz") && (source_option.size()!=8) )
	){
      int nopt_req=7;
      if(method=="poisson_anz") nopt_req=8;
      cout<<"============== ERROR =============== "<<endl;
      cout<<"You have requested the "<<method
	  <<" overlay option which requires "<<nopt_req<<" fields"<<endl;
      
      cout<<"This one "<<opt_arg<<" has "<<source_option.size()<<" fields"<<endl;
      cout<<"ERROR: We can't continue!"<<endl;
      option::printUsage(std::cout, usage);
      return 1;      
    }
    
    // determine the rskip option
    bool rskip=false;
    if(source_option[4]=="y") rskip=true;
    else if (source_option[4]=="n") rskip=false;
    else{
      cout<<"============== ERROR =============== "<<endl;
      cout<<"The third field in a source command needs to be y or n"<<endl;
      cout<<"Yours is: "<<source_option[4]<<endl;
      cout<<"ERROR: We can't continue!"<<endl;
      option::printUsage(std::cout, usage);
      return 1;      
    }
    
    // decode the mu or n field, field 4
    string field6=source_option[6];
    int n=-1; float mu=-1;
    if ( method=="fixed" ){
      bool is_non_neg_int = (field6.find_first_not_of( "0123456789" ) == string::npos);
      if(is_non_neg_int) {
	char *end;
	n=std::strtol(field6.c_str(),&end,10);
      }
      else{
	cout<<"============== ERROR =============== "<<endl;
	cout<<"The seventh field in a source command needs to be positive integer when using fixed method"<<endl;
	cout<<"Yours is: "<<field6<<endl;
	cout<<"ERROR: We can't continue!"<<endl;
	option::printUsage(std::cout, usage);
	return 1;
      }
    }            
    else if( (method=="poisson") || (method=="poisson_anz") ){
      bool is_pos_float = (field6.find_first_not_of( ".0123456789" ) == string::npos);
      if(is_pos_float) {
	char *end;
	mu=std::strtof(field6.c_str(),&end);
      }
      else{
	cout<<"============== ERROR =============== "<<endl;
	cout<<"The seventh field in a source command needs to be positive float when using a poisson method"<<endl;
	cout<<"Yours is: "<<field6<<endl;
	cout<<"ERROR: We can't continue!"<<endl;
	option::printUsage(std::cout, usage);
	return 1;
      }      
    }

    // decode the frac field if we are doing poisson_anz
    float frac=-1.0;
    if(method=="poisson_anz"){
      string field7=source_option[7];
      bool is_pos_float = (field7.find_first_not_of( ".0123456789" ) == string::npos);
      char *end;
      if(is_pos_float) {frac=std::strtof(field7.c_str(),&end);}
      if(!is_pos_float || frac<0.0 || frac>1.0){       
	cout<<"============== ERROR =============== "<<endl;
	cout<<"The eighth field in a source command needs to be positive float on [0,1] when using the poisson_anz method"<<endl;
	cout<<"Yours is: "<<field7<<endl;
	cout<<"ERROR: We can't continue!"<<endl;
	option::printUsage(std::cout, usage);
	return 1;
      }     
    }

    // read the seed field and check it
    int seed=-1;
    string field2=source_option[2];
    bool seed_is_pos_int = (field2.find_first_not_of( "0123456789" ) == string::npos);
    char *end;
    if(seed_is_pos_int) {seed=std::strtol(field2.c_str(),&end,10);}
    else{
	cout<<"============== ERROR =============== "<<endl;
	cout<<"The second field (seed) in a source command needs to be positive int"<<endl;
	cout<<"Yours is: "<<field2<<endl;
	cout<<"ERROR: We can't continue!"<<endl;
	option::printUsage(std::cout, usage);
	return 1;
    }     

    // read the rmax field and check it
    int rmax=-1;
    string field3=source_option[3];
    bool rmax_is_pos_int = (field3.find_first_not_of( "0123456789" ) == string::npos);    
    if(rmax_is_pos_int) {rmax=std::strtol(field3.c_str(),&end,10);}
    else{
	cout<<"============== ERROR =============== "<<endl;
	cout<<"The third field (rmax) in a source command needs to be positive int"<<endl;
	cout<<"Yours is: "<<field3<<endl;
	cout<<"ERROR: We can't continue!"<<endl;
	option::printUsage(std::cout, usage);
	return 1;
    }     
    

    

    cout<<"============= New Neutrino Source "<<cntr<<" ================="<<endl;
    cout<<"Method: "<<method;
    if(method=="fixed") cout<<" with N="<<n<<" events/spill"<<endl;
    if(method=="poisson") cout<<" with average mu="<<mu<<" events/spill"<<endl;
    if(method=="poisson_anz") {
      cout<<" with average  mu="<<mu<<" events/spill\n";
      cout<<"        "<<frac*100<<"% of spills assured to have 1 or more events"<<endl;
    }
    cout<<"Random seed: "<<seed<<endl;
    cout<<"Will quit when any one event will be reused > "<<rmax<<" times"<<endl;
    if(rskip) cout<<"Will randomly skip through the source file(s)"<<endl;
    else cout<<"Will read events from the source file(s) in order"<<endl;
    cout<<"Source ntuple name: "<<source_option[1]<<endl;
    cout<<"Source files:\n "<<source_option[0]<<endl;
    cout<<"Reading files now........"<<endl;
    TChain* ntuple= new TChain(source_option[1].c_str(),source_option[1].c_str());
    int nfiles=ntuple->Add(source_option[0].c_str(),-1);
    if(nfiles==0){
      cout<<"ERROR: could not find any files! Aborting."<<endl;
      return 1;      
    }
    cout<<"Found "<<nfiles<<" files. List follows:"<<endl;
    ntuple->ls();
    cout<<"============= Done adding source "<<cntr<<" ==================\n"<<endl;
    // now create the source
    event_source* es = new event_source(ntuple,seed,rmax,rskip);
    event_sources.push_back(es);
    event_sources_opts.push_back( source_opts(method,mu,n,frac) );
  }
  //////////////// done creating event sources ///////////////////
  
  /////////////////// make an output file ////////////////////////
  TFile fout(output_filename.c_str(),"RECREATE");
  TTree* tout= new TTree(output_ntuple.c_str(),"GENIE MC Truth TTree, Format: [NtpMCEventRecord]");
  NtpMCEventRecord* gmcrec = new NtpMCEventRecord();
  tout->Branch("gmcrec",&gmcrec,32000,0);
  /////////////////// makes some spills //////////////////////////
  cout<<"============== Now constructing spills ============="<<endl;
  int ispill=0;
  vector<int> total_evts_per_source(event_sources.size(),0);
  for(; ispill<nspill; ispill++){
    cout<<"Spill "<<ispill<<endl;
    
    events spill_events;
    vector<int> spill_events_per_source(event_sources.size(),0);
    try{
      for(uint isource=0; isource<event_sources.size(); isource++){
	event_source* es = event_sources[isource];
	source_opts so = event_sources_opts[isource];
	events evs;
	float weight=1.0;
	if(so.method=="poisson"){
	  evs=es->get_poisson(so.mu);
	}
	else if(so.method=="poisson_anz"){
	  evs=es->get_poisson_assure_nonzero(so.mu,weight,so.frac);
	  for(uint ievt=0; ievt<evs.size(); ievt++) evs[ievt]->event->SetWeight(weight);
	}
	else if(so.method=="fixed"){
	  evs=es->get_fixed(so.n);
	}
	cout<<" Source "<<isource<<" yielded "<<evs.size()<<" events"<<endl;
	spill_events.insert(spill_events.end(),evs.begin(),evs.end());
	spill_events_per_source[isource]=evs.size();
      }
      cout<<" Total of "<<spill_events.size()<<" events for spill "<<ispill<<endl;
      for(uint isource=0; isource<event_sources.size(); isource++){
	total_evts_per_source[isource]+=spill_events_per_source[isource];
      }
    }
    catch(const all_done& ex){
      cout<<"=============== early end of spill generation ======================="<<endl;
      cout<<"Overlaying ends before completion of spill "<<ispill<<endl;
      cout<<"Reason: "<<ex.what()<<endl;
      cout<<"We will complete writing of output ntuples and terminate."<<ex.what()<<endl;
      ispill--;
      break;
    }
    // do time distribution and ordering
    if (time_profile!=0){ // we have a spill profile histogram
      event_source* es = event_sources[0]; // could use any of them
      es->distribute_in_time(spill_events, time_profile);
      event_source::time_order(spill_events);
    }
    else if(tstart!=tend){// user must have set these
      event_source* es = event_sources[0]; // could use any of them
      es->distribute_in_time(spill_events, tstart,tend);
      event_source::time_order(spill_events);
    }

    
    // put a dummy event in to mark the end of the spill
    // the initial state has one particle, a rootino (ipdg=0)
    event* dummy_event= event_source::make_spill_end_marker(0,20e3);
    cout<<"================== created dummy event =============="<<endl;
    cout<<*dummy_event<<endl;
    spill_events.push_back(dummy_event);
    // copy spill into output tree
    for(uint iev=0; iev<spill_events.size(); iev++){
      NtpMCEventRecord* current_event=spill_events[iev];
      gmcrec->Copy(*current_event);
      tout->Fill();
    }

  }
  cout<<"=============== done with making spills ======================="<<endl;
  cout<<" Overlayed "<<ispill<<" out of the "<<nspill<<" requested spills"<<endl;
  cout<<" Events used:"<<endl;
  for(uint isource=0; isource<event_sources.size(); isource++){
    cout<<"   Source "<<isource+1<<": "<<total_evts_per_source[isource]<<endl;;
  }
  
    

  // make the a tree header and add it to the output
  NtpMCTreeHeader* tree_header= new NtpMCTreeHeader();
  tree_header->format=genie::kNFGHEP;
  fout.cd();
  tree_header->Write("header");
  fout.Write();
  fout.Close();
  return 0;
}
