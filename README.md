# OverlayGenie
OverlayGenie is a program that takes events from multiple genie ghep ntuples (event sources) and creates an overlay ntuple, combining the different event sources in a way that the user can control. This is useful for simulating neutrino beam spills in the case that some elements of the geometry are much less dense but important to have events in (with weighting) or the alternative case in which some of the geometry is just generating background events that could be reused to make the generation more efficient.
## Dependencies
The only external dependencies are ROOT and GENIE. Commandline options are handled by [optionparser]( http://optionparser.sourceforge.net/ "Lean Mean C++ Option Parser page on SourceForge") which is included via a header file.

The program was developed against GENIE v2.12.6 and ROOT 6.08/06. The development platform was a Linux 2.6 64-bit system with GCC 6.3.0.  Other reasonably new versions of GENIE, ROOT and GCC are expected to work. The code is written in C/C++ but doesn't take advantage of much from the C++11 and new standards.

## Features
1. Any number of input `TTrees` or `TChains`. These become affiliated with `event_source` objects.
2. Each `event_source` can be configured to pull events according to
  * A fixed number per spill: ~fixed~
  * A Poisson distribution: ~poisson~
  * A Poisson distribution with >0 events: ~poisson_anz~ (assure non-zero). This method also provides a weight which can be used at analysis time to deweight such events.
3. The program can randomly scatter events time to simulate a beam profile, with two options:
  * Uniformly in a time range.
  * Or according to a `ROOT::TH1`.
  In either case the events are later time ordered.
4. Events from each source can be reused as many times as is desired.
5. The event sources can be read and used linearly or randomly.
6. The output is a single ROOT ntuple file containing one GENIE ghep event in each entry. Spills endings are marked by dummy events containing a single "Rootino" (`ipdg==0`).
## A spill ending dummy event
```
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
