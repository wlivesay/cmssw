Notation for these files:
TTBarPU0H500T0N10N13 explanation

Type of interaction (TTbar)
Pileup (PU0 or PU200)
H,HD,HDnKFM,HDnKFK (Hybrid, Hybrid Displaced, Hybrid Displaced new KF Merge/Kill)
	I dont think HR (Hybrid Reduced) is working so I haven't tried it
Then number of events (500, 20k, 50k...)
Then Truncation:
	If not included assume T=0
	T is actually the limit on truncation so T=0 means full truncation applied! T=10k means lets 10k tracks
		through before truncating. For hybrid auto is T=0 but I think for Displaced auto is 10k (complicated)
N (1)
	If not included assume N1=7
	This has something to do with 2^N1 = 128 rn so 127 is cap. If we raise this we could change T up higher than
		127 but N2 must also compensate. (Note I ran into other issues with 127 being hardcoded so it might
		be best to stay away from changing N1,N2
	Found in interface/Settings.h file
N (2)
	If not included assume N2=10
	This one I think has to be N1+3 from experience
	Found in src/Residual.cc file
