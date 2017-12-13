# OverlayGenie
OverlayGenie is a program that takes events from multiple genie ghep ntuples (event sources) and creates an overlay ntuple, combining the different event sources in a way that the user can control. This is useful for simulating neutrino beam spills in the case that some elements of the geometry are much less dense but important to have events in (with weighting) or the alternative case in which some of the geometry is just generating background events that could be reused to make the generation more efficient.
## Dependencies
The only external dependencies are ROOT and GENIE. Commandline options are handled by [optionparser]( http://optionparser.sourceforge.net/ "Lean Mean C++ Option Parser page on SourceForge") which is included via a header file.

The program was developed against GENIE v2.12.6 and ROOT 6.08/06. The development platform was a Linux 2.6 64-bit system with GCC 6.3.0.  Other reasonably new versions of GENIE, ROOT and GCC are expected to work. The code is written in C/C++ but doesn't take advantage of much from the C++11 and new standards.

## Features
1. Any number of input ROOT `TTrees` or `TChains` containing GENIE ghep records. These become affiliated with `event_source` objects.
2. Each `event_source` can be configured to pull events according to
  * A fixed number per spill: *fixed*
  * A Poisson distribution: *poisson*
  * A Poisson distribution with >0 events: *poisson_anz* (assure non-zero). This method also provides a weight which can be used at analysis time to deweight such events.
3. The program can randomly scatter events time to simulate a beam profile, with two options:
  * Uniformly in a time range.
  * Or according to a ROOT `TH1`.
  In either case the events are later time ordered.
4. Events from each source can be reused as many times as is desired. 
5. The event sources can be read and used linearly or randomly.
## Output and stopping condition

The output is a single ROOT ntuple file containing one GENIE ghep event in each entry. Spills endings are marked by dummy events containing a single "Rootino" (`ipdg==0`). (see example below)

The program exits after:
1. generating the requested number of spills 
2. or just before reusing an event more than the requested number of times.
3. or when one of the sources runs out of events

In the latter two cases the final (incomplete) spill is not generated.

## Building the code.
You need GENIE and ROOT setup with their usual environmental variables defined. Then simply type `make` to build the `overlay_genie` program.

## `overlay_genie`
It's a simple command line program.  

```
./overlay_genie -h
USAGE:  overlay_genie [options]

NOTE: short option flags must not have a space between the flag and the option
NOTE: long option flags must have an = between the flag and the option
      -xmyoption is OK as is --longx=myoption
      -x myoption fails as does --longx myoption
Options:
 -h --help     Print usage and exit.
 --source, -s   specify a neutrino source and overlay options
 Format:
 -s/full/path/to/file.root,tree_name,seed,rmax,rskip,overlay_meth,overlay_par(s)
    * seed (int) is a seed to be passed to the RNG (TRandom3)
    * rmax (int) is the maximum # of times an event can be reused
    * rskip y or n specifies that the file is read randomly (y) or linearly (n)
    * overlay_method and overlay_par(s) are one of
      - fixed,<nevents> (integer)
          choose a fixed number of events for each spill
      - poisson,<mu> (float)
          draw from a Poisson distribution with parameter <mu> events per spill
      - poisson_anz,<mu>,<frac> (float),(float)
          like poisson but <frac> of the time generate a non-zero result
          distributed according to a Poisson with parameter <mu>
          the method doing this also returns a weight which it 
          records in the GHEPRecord::Weight() for all events 
          created by this poisson_anz source for a given spill
 -o --output outputfile.root,ntuplename
 -R --time_range (float),(float)
 -H --time_hist rootfile,TH1_name
 -n --nspills (integer)
      Note: we cannot guarantee nspills as one of the sources
      may run out first. It is a target/maximum.
```
## Example invocation
```
./overlay_genie \
--source=/full/path/to/ntuplesA_[0-2].ghep.root,gtree,2718,10,y,poisson,50 \  
--source=/full/path/to/ntupleB.ghep.root,gtree,314159,1,n,fixed,1  \
--source=/full/path/to/ntupleC.ghep.root,gtree,1234,1,y,poisson_anz,0.13,0.65 \
--nspills=3 --output=overlay_genie.root,gtree --time_range=1100.0,10900
```
This overlays events from three sources. Each source contains `TTrees` with the name `ghep` (usual GENIE standard).

### Source 1
*This is an example of how one might use a source of background events, perhaps from the cavern rock.*
`--source=/full/path/to/ntuplesA_[0-2].ghep.root,gtree,2718,10,y,poisson,50`
* Consists of three files. The regular expression syntax will be interpreted by `TChain::Add()`. 
* The random seed used to initialize the source's `TRandom3` generator is 2718
* Events can be used up to 10 times.
* The file will be read randomly (`y` option)
* Events will be pulled according to a Poisson distribution with mu=50.

### Source 2
`--source=/full/path/to/ntupleB.ghep.root,gtree,314159,1,n,fixed,1`
* Consists of a single file.
* The seed is 314159.
* Events can be reused only once and are read linearly (there would be no point in reading them randomly but one could request it).
* One event will be pulled in each spill (the final `1` above).

### Source 3
*This is an example of how one might use a source of signal events occuring in a low mass detector*
`--source=/full/path/to/ntupleC.ghep.root,gtree,1234,1,y,poisson_anz,0.13,0.65`
* Consists of a single file.
* The seed is 1234.
* Events can be reused only once and are read linearly.
* Events are pulled according to a Poisson distribution with mu=0.13. However 65% of the time (the final `0.65` above) the program will guarantee that at least one event is pulled. In that case an event weight will be provided and written into the output ntuple (see below for details).

### Other options
* Three spills are requested and will be generated unless one of the other stopping conditions is encountered (see above).
* The output is a file `overlay_genie.root` with ghep ntuple inside called `gtree`.
* Events in each spill will be scattered uniformly in a time range of 1100ns to 10900ns. 

One could alternatively specify a histogram that will be sampled by `TH1::GetRandom()` to simulate the time profile:

This option `--time_hist=spill_profile.root,my_hist` would cause the program to try and use a histogram called `my_hist` in the file spill_profile.root (contained in the working directory in this example, but a full path can also be specified). 




## A spill ending dummy event
|------------------------------------------------------------------------------------------------------------------|
|GENIE GHEP Event Record [print level:   3]                                                                        |
|------------------------------------------------------------------------------------------------------------------|
| Idx |          Name | Ist |        PDG |   Mother  | Daughter  |      Px |      Py |      Pz |       E |      m  | 
|------------------------------------------------------------------------------------------------------------------|
|   0 |       Rootino |  -1 |          0 |   0 |   0 |   0 |   0 |   0.000 |   0.000 |   0.000 |   0.000 |   0.000 | 
|------------------------------------------------------------------------------------------------------------------|
|       Fin-Init:                                                |   0.000 |   0.000 |   0.000 |   0.000 |         | 
|------------------------------------------------------------------------------------------------------------------|
| Err flag [bits:15->0] : 0000000000000000    |  1st set:                                                     none | 
| Err mask [bits:15->0] : 1111111111111111    |  Is unphysical:    NO |   Accepted:   YES                          |
|------------------------------------------------------------------------------------------------------------------|
| sig(Ev) =       0.00000e+00 cm^2  | dsig(Ev;{K_s})/dK   =     0.00000e+00 cm^2/{K}   | Weight =          1.00000 |
|------------------------------------------------------------------------------------------------------------------|

--------------------------------------------------------------------------------------------------------------
GENIE Interaction Summary
--------------------------------------------------------------------------------------------------------------
[-] [Init-State] 
 |--> probe        : PDG-code = 0 (Rootino)
 |--> nucl. target : Z = 0, A = 0, PDG-Code = 0 (Rootino)
 |--> hit nucleon  : no set
 |--> hit quark    : no set
 |--> probe 4P     : (E =     0.000000, Px =     0.000000, Py =     0.000000, Pz =     0.000000)
 |--> target 4P    : (E =     0.000000, Px =     0.000000, Py =     0.000000, Pz =     0.000000)

[-] [Process-Info]  
 |--> Interaction : Unknown
 |--> Scattering  : Unknown
[-] [Kinematics]
[-] [Exclusive Process Info] 
 |--> charm prod.  : false |--> strange prod.  : false
 |--> f/s nucleons : N(p) = 0 N(n) = 0
 |--> f/s pions    : N(pi^0) = 0 N(pi^+) = 0 N(pi^-) = 0
 |--> resonance    : [not set]
--------------------------------------------------------------------------------------------------------------
```
