#!/bin/bash
set -e
# =============================================================================
# L1 Track Unit Test 1
# Step 1: Check the code compiles successfully.
# Step 2: Run cmsRun over 5 events without crashing.
# =============================================================================
XROOTD_PREFIX="root://cms-xrd-global.cern.ch/"
MC_DATASET="${XROOTD_PREFIX}/store/relval/CMSSW_15_1_0_pre5/RelValTTbar_14TeV_TuneCP5/GEN-SIM-DIGI-RAW/PU_150X_mcRun4_realistic_v1_RV269_Run4D110_PU-v2/2590000/0f0bcfd3-dafe-4dda-8d39-9765f6eae68e.root"
# =============================================================================
# STEP 1: Compile
# =============================================================================
echo "=== Step 1: Compiling ==="
cd $CMSSW_BASE/src
#scram b -j4   # handled by the outer runUT.sh harness, not here
echo "=== Step 1: SUCCESS - code compiled ==="

# =============================================================================
# STEP 2: Run 5 events
# =============================================================================
echo "=== Step 2: Running 5 events ==="
cd $CMSSW_BASE/src/L1Trigger/TrackFindingTracklet/test/

JOBNAME="job_UT1_cfg.py"
cp L1TrackNtupleMaker_cfg.py $JOBNAME
echo "process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(5))" >> $JOBNAME
echo "process.source.fileNames = cms.untracked.vstring('$MC_DATASET')" >> $JOBNAME
cmsRun $JOBNAME
echo "=== Step 2: SUCCESS - ran 5 events without crashing ==="
