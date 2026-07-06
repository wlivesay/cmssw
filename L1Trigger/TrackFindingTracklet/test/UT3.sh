#!/bin/bash
trap 'rm -f mc_dataset.root $JOBNAME' EXIT
set -e
# =============================================================================
# L1 Track Unit Test 3
# Step 1: Check the code compiles successfully.
# Step 2: Run cmsRun over 5 events without crashing.
# Step 3: Run makeHists.csh and confirm it completes without errors.
# Step 4: Check coding style (auto-formatter).
#
# NOTE: Step 4 is handled automatically by cms-bot for every PR.
# It is included here for local testing only.
#
# HOW TO USE:
#   cd ~/CMSSW_15_1_0_pre4/src
#   cmsenv
#   bash unit_test_2p5.sh
# =============================================================================
# -----------------------------------------------------------------------------
# Dataset: TTbar, PU0, 1000 events (CERNBox skim, skimmedForCI_15_1_0.root)
# PU0 is intentional here -- this is a fast CI smoke test (compiles + runs
# without crashing), NOT a PU200 physics-performance validation. Real PU200
# validation is handled separately (see UT1.sh / l1track_ci.sh), since a live
# PU200 grid fetch is too slow/variable to run on every PR.
#
# GRID PU200 alternative (reference values would need updating if switched):
# XROOTD_PREFIX="root://cms-xrd-global.cern.ch/"
# MC_DATASET="${XROOTD_PREFIX}/store/relval/CMSSW_15_1_0_pre5/RelValTTbar_14TeV_TuneCP5/GEN-SIM-DIGI-RAW/PU_150X_mcRun4_realistic_v1_RV269_Run4D110_PU-v2/2590000/0f0bcfd3-dafe-4dda-8d39-9765f6eae68e.root"
# -----------------------------------------------------------------------------
MC_DATASET="https://cernbox.cern.ch/remote.php/dav/public-files/PjrWRBYNJq7OOzi/skimmedForCI_15_1_0.root"
TEST_DIR="$CMSSW_BASE/src/L1Trigger/TrackFindingTracklet/test"

# =============================================================================
# SANITY CHECK
# =============================================================================
if [ -z "$CMSSW_BASE" ]; then
    echo "ERROR: CMSSW_BASE is not set. Run cmsenv before this script."
    exit 1
fi

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

JOBNAME="job_UT2p5_cfg.py"
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

# =============================================================================
# STEP 4: Code style check - auto-formatter
# Runs scram b code-format on the checked out packages.
# Fails if any files were modified by the formatter (meaning the developer
# forgot to run it before submitting).
# NOTE: cms-bot runs this automatically on every PR - included here for
# local verification only.
# =============================================================================
# =============================================================================
# STEP 4: Code style check - auto-formatter
# Runs scram b code-format on the checked out packages.
# Fails only if code-format ITSELF modifies files during this run (meaning
# the developer forgot to run it before submitting). Uses a before/after
# snapshot of `git status` rather than a single post-hoc check, so
# pre-existing unrelated changes already sitting in the working tree (e.g.
# BuildFile.xml, edited by runUT.sh before this test ever started) aren't
# mistaken for formatter issues.
# NOTE: cms-bot runs this automatically on every PR - included here for
# local verification only.
# =============================================================================
echo "=== Step 4: Checking coding style (formatter) ==="
cd $CMSSW_BASE/src
export USER_CODE_CHECKS_ARGS='--quiet'

BEFORE=$(git status --porcelain)
scram b -j code-format
AFTER=$(git status --porcelain)

if [ "$BEFORE" != "$AFTER" ]; then
    echo "FAILURE - 'scram b -j code-format' modified files; please run it and commit before submitting PR."
    git status
    exit 1
else
    echo "SUCCESS - code formatting is clean."
fi

