#This will be my Crabb configuration python file!
#Make sure to edit this file with all the correct information
#Also go to L1TrackNtupleMaker_cfg.py and comment out all inputMC datasets
#Should probably set openpy to maxEvents of -1 so that isn't the limiter


from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'test1'          # creates crab_test1/ directory
config.General.transferOutputs = True
config.General.transferLogs = True

config.JobType.pluginName = 'Analysis'
config.JobType.psetName   = '../../L1TrackNtupleMaker_cfg.py'   # your existing config here

config.Data.inputDataset = ''  # replace with your DAS dataset
config.Data.inputDBS     = 'global'
config.Data.splitting    = 'FileBased' #Tells the next line what to consider a 'unit' as!!!
#Options for below are:
	#'FileBased' (use for MC/simulation)
	#'Lumibased' (certain number of particle bunches ~23seconds!!)
	#'EventAwareLumiBased' (what I am used to)
config.Data.unitsPerJob  = 5 #right now this says 5 Root files
config.Data.lumiMask     = 'my_lumi_mask.json'     # remove this line if running on MC

config.Site.storageSite  = ''             # replace with your site
