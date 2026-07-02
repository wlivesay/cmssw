#!/bin/bash
set -e

# =============================================================================
# L1 Track Unit Test 2
# Step 1: Check the code compiles successfully.
# Step 2: Run cmsRun over 5 events without crashing.
# Step 3: Run makeHists.csh and confirm it completes without errors.
# =============================================================================

XROOTD_PREFIX="root://cms-xrd-global.cern.ch/"
MC_DATASET="${XROOTD_PREFIX}/store/relval/CMSSW_15_1_0_pre5/RelValTTbar_14TeV_TuneCP5/GEN-SIM-DIGI-RAW/PU_150X_mcRun4_realistic_v1_RV269_Run4D110_PU-v2/2590000/0f0bcfd3-dafe-4dda-8d39-9765f6eae68e.root"

TEST_DIR="$CMSSW_BASE/src/L1Trigger/TrackFindingTracklet/test"

# =============================================================================
# STEP 1: Compile
# =============================================================================
echo "=== Step 1: Compiling ==="
cd $CMSSW_BASE/src
scram b -j4
echo "=== Step 1: SUCCESS — code compiled ==="

# =============================================================================
# STEP 2: Run 5 events
# =============================================================================
echo "=== Step 2: Running 5 events ==="
cd $TEST_DIR
cmsRun L1TrackNtupleMaker_cfg.py maxEvents=5 inputFiles=$MC_DATASET
echo "=== Step 2: SUCCESS — ran 5 events without crashing ==="

# =============================================================================
# STEP 3: Run makeHists.csh
# =============================================================================
echo "=== Step 3: Running makeHists.csh ==="
tcsh makeHists.csh L1TrkNtuple.root

if [ -f "results.out" ]; then
    echo "=== Step 3: SUCCESS — makeHists.csh completed and produced results.out ==="
    echo "=== results.out contents: ==="
else
    echo "=== Step 3: FAILURE — results.out not produced ==="
    exit 1
fi
