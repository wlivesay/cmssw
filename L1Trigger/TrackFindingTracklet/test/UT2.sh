#!/bin/bash
trap 'rm -f mc_dataset.root $JOBNAME' EXIT
set -e
# =============================================================================
# L1 Track Unit Test 2
# Step 1: Check the code compiles successfully.
# Step 2: Run cmsRun over 5 events without crashing.
# Step 3: Run makeHists.csh and confirm it completes without errors.
#
# HOW TO USE:
#   cd ~/CMSSW_15_1_0_pre4/src
#   cmsenv
#   bash unit_test_2.sh
# =============================================================================
# TTbar PU0, 1000 events (CERNBox skim, skimmedForCI_15_1_0.root)
# Matches the reference values used in unit_test_3.sh
# GRID PU200 alternative (reference values will need updating if switched):
# XROOTD_PREFIX="root://cms-xrd-global.cern.ch/"
# MC_DATASET="${XROOTD_PREFIX}/store/relval/CMSSW_15_1_0_pre5/RelValTTbar_14TeV_TuneCP5/GEN-SIM-DIGI-RAW/PU_150X_mcRun4..."
MC_DATASET="https://cernbox.cern.ch/remote.php/dav/public-files/PjrWRBYNJq7OOzi/skimmedForCI_15_1_0.root"
TEST_DIR="$CMSSW_BASE/src/L1Trigger/TrackFindingTracklet/test"

# =============================================================================
# STEP 1: Compile
# =============================================================================
echo "=== Step 1: Compiling ==="
cd $CMSSW_BASE/src
scram b -j4
echo "=== Step 1: SUCCESS - code compiled ==="

# =============================================================================
# STEP 2: Run 5 events
# =============================================================================
echo "=== Step 2: Running 5 events ==="
cd $TEST_DIR
curl -k -o mc_dataset.root $MC_DATASET

JOBNAME="job_UT2_cfg.py"
cp L1TrackNtupleMaker_cfg.py $JOBNAME
echo "process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(5))" >> $JOBNAME
echo "process.source.fileNames = cms.untracked.vstring('file:mc_dataset.root')" >> $JOBNAME
cmsRun $JOBNAME
echo "=== Step 2: SUCCESS - ran 5 events without crashing ==="

# =============================================================================
# STEP 3: Run makeHists.csh
# =============================================================================
echo "=== Step 3: Running makeHists.csh ==="
tcsh makeHists.csh L1TrkNtuple.root
if [ -f "results.out" ]; then
    echo "=== Step 3: SUCCESS - makeHists.csh completed and produced results.out ==="
else
    echo "=== Step 3: FAILURE - results.out not produced ==="
    exit 1
fi
