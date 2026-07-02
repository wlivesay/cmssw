#!/bin/bash
set -e

# =============================================================================
# L1 Track Unit Test 3
# Step 1: Check the code compiles successfully.
# Step 2: Run cmsRun over 5 events without crashing.
# Step 3: Run makeHists.csh and confirm results.out is produced.
# Step 4: Validate physics output against reference thresholds.
#
# HOW TO USE:
#   cd ~/CMSSW_15_1_0_pre4/src
#   cmsenv
#   bash ~/CMSSW_15_1_0_pre4/src/L1Trigger/TrackFindingTracklet/test/Billy/Projects/unitTest/localUnitTest3.sh
#
# Set QUICK_TEST="false" to run all three algorithms (slower).
# =============================================================================

# =============================================================================
# CONFIGURATION
# =============================================================================
QUICK_TEST="true"   # "true" = HYBRID only; "false" = all three algorithms
CFG_FILE="L1TrackNtupleMaker_cfg.py"
TEST_DIR="$CMSSW_BASE/src/L1Trigger/TrackFindingTracklet/test"

MC_DATASET_TTBAR="https://cernbox.cern.ch/remote.php/dav/public-files/PjrWRBYNJq7OOzi/skimmedForCI_15_1_0.root"
# GRID PU200 alternative (reference values will need updating if you switch to this):
# MC_DATASET_TTBAR="${XROOTD_PREFIX}/store/relval/CMSSW_15_1_0_pre5/RelValTTbar_14TeV_TuneCP5/GEN-SIM-DIGI-RAW/PU_150X_mcRun4_realistic_v1_RV269_Run4D110_PU-v2/2590000/0f0bcfd3-dafe-4dda-8d39-9765f6eae68e.root"

# =============================================================================
# REFERENCE AND THRESHOLD VALUES
# Reference = last known good run value
# Threshold = pass/fail boundary (3-sigma from reference)
# =============================================================================

# --- HYBRID ---
HYBRID_EFFI_ETA_LOW_REFERENCE="96.88"       HYBRID_EFFI_ETA_LOW_THRESHOLD="96.40"
HYBRID_EFFI_ETA_MID_REFERENCE="95.95"       HYBRID_EFFI_ETA_MID_THRESHOLD="95.23"
HYBRID_EFFI_ETA_HIGH_REFERENCE="97.22"      HYBRID_EFFI_ETA_HIGH_THRESHOLD="96.41"
HYBRID_EFFI_PT2_REFERENCE="96.66"           HYBRID_EFFI_PT2_THRESHOLD="96.30"
HYBRID_EFFI_PT2TO8_REFERENCE="96.47"        HYBRID_EFFI_PT2TO8_THRESHOLD="96.05"
HYBRID_EFFI_PT8_REFERENCE="97.23"           HYBRID_EFFI_PT8_THRESHOLD="96.60"
HYBRID_EFFI_PT40_REFERENCE="95.82"          HYBRID_EFFI_PT40_THRESHOLD="93.10"
HYBRID_TP_PT2_REFERENCE="22.66"             HYBRID_TP_PT2_THRESHOLD="22.21"
HYBRID_TP_PT3_REFERENCE="15.69"             HYBRID_TP_PT3_THRESHOLD="15.31"
HYBRID_TP_PT10_REFERENCE="4.44"             HYBRID_TP_PT10_THRESHOLD="4.24"
HYBRID_NTRK_PT2_REFERENCE="28.22"           HYBRID_NTRK_PT2_THRESHOLD="28.72"
HYBRID_NTRK_PT3_REFERENCE="20.23"           HYBRID_NTRK_PT3_THRESHOLD="20.66"
HYBRID_NTRK_PT10_REFERENCE="6.06"           HYBRID_NTRK_PT10_THRESHOLD="6.29"
HYBRID_FAKE_REFERENCE="5.54"                HYBRID_FAKE_THRESHOLD="5.95"
HYBRID_DUP_REFERENCE="3.97"                 HYBRID_DUP_THRESHOLD="4.32"
HYBRID_Z0RES_LOWETA_REFERENCE="0.09"        HYBRID_Z0RES_LOWETA_THRESHOLD="0.096"
HYBRID_Z0RES_HIGHETA_REFERENCE="0.38"       HYBRID_Z0RES_HIGHETA_THRESHOLD="0.405"

# --- HYBRID_NEWKF ---
HYBRID_NEWKF_EFFI_ETA_LOW_REFERENCE="97.32"     HYBRID_NEWKF_EFFI_ETA_LOW_THRESHOLD="96.87"
HYBRID_NEWKF_EFFI_ETA_MID_REFERENCE="95.95"     HYBRID_NEWKF_EFFI_ETA_MID_THRESHOLD="95.23"
HYBRID_NEWKF_EFFI_ETA_HIGH_REFERENCE="97.48"    HYBRID_NEWKF_EFFI_ETA_HIGH_THRESHOLD="96.70"
HYBRID_NEWKF_EFFI_PT2_REFERENCE="96.94"         HYBRID_NEWKF_EFFI_PT2_THRESHOLD="96.61"
HYBRID_NEWKF_EFFI_PT2TO8_REFERENCE="96.97"      HYBRID_NEWKF_EFFI_PT2TO8_THRESHOLD="96.58"
HYBRID_NEWKF_EFFI_PT8_REFERENCE="96.86"         HYBRID_NEWKF_EFFI_PT8_THRESHOLD="96.17"
HYBRID_NEWKF_EFFI_PT40_REFERENCE="96.03"        HYBRID_NEWKF_EFFI_PT40_THRESHOLD="93.36"
HYBRID_NEWKF_TP_PT2_REFERENCE="22.66"           HYBRID_NEWKF_TP_PT2_THRESHOLD="22.21"
HYBRID_NEWKF_TP_PT3_REFERENCE="15.69"           HYBRID_NEWKF_TP_PT3_THRESHOLD="15.31"
HYBRID_NEWKF_TP_PT10_REFERENCE="4.44"           HYBRID_NEWKF_TP_PT10_THRESHOLD="4.24"
HYBRID_NEWKF_NTRK_PT2_REFERENCE="29.58"         HYBRID_NEWKF_NTRK_PT2_THRESHOLD="30.10"
HYBRID_NEWKF_NTRK_PT3_REFERENCE="21.50"         HYBRID_NEWKF_NTRK_PT3_THRESHOLD="21.94"
HYBRID_NEWKF_NTRK_PT10_REFERENCE="6.74"         HYBRID_NEWKF_NTRK_PT10_THRESHOLD="6.99"
HYBRID_NEWKF_FAKE_REFERENCE="9.93"              HYBRID_NEWKF_FAKE_THRESHOLD="10.45"
HYBRID_NEWKF_DUP_REFERENCE="3.66"               HYBRID_NEWKF_DUP_THRESHOLD="3.99"
HYBRID_NEWKF_Z0RES_LOWETA_REFERENCE="0.10"      HYBRID_NEWKF_Z0RES_LOWETA_THRESHOLD="0.107"
HYBRID_NEWKF_Z0RES_HIGHETA_REFERENCE="0.44"     HYBRID_NEWKF_Z0RES_HIGHETA_THRESHOLD="0.47"

# --- HYBRID_DISPLACED ---
HYBRID_DISPLACED_EFFI_ETA_LOW_REFERENCE="97.84"     HYBRID_DISPLACED_EFFI_ETA_LOW_THRESHOLD="97.45"
HYBRID_DISPLACED_EFFI_ETA_MID_REFERENCE="96.65"     HYBRID_DISPLACED_EFFI_ETA_MID_THRESHOLD="95.99"
HYBRID_DISPLACED_EFFI_ETA_HIGH_REFERENCE="97.40"    HYBRID_DISPLACED_EFFI_ETA_HIGH_THRESHOLD="96.62"
HYBRID_DISPLACED_EFFI_PT2_REFERENCE="97.41"         HYBRID_DISPLACED_EFFI_PT2_THRESHOLD="97.08"
HYBRID_DISPLACED_EFFI_PT2TO8_REFERENCE="97.34"      HYBRID_DISPLACED_EFFI_PT2TO8_THRESHOLD="96.98"
HYBRID_DISPLACED_EFFI_PT8_REFERENCE="97.61"         HYBRID_DISPLACED_EFFI_PT8_THRESHOLD="97.01"
HYBRID_DISPLACED_EFFI_PT40_REFERENCE="96.45"        HYBRID_DISPLACED_EFFI_PT40_THRESHOLD="93.90"
HYBRID_DISPLACED_TP_PT2_REFERENCE="22.66"           HYBRID_DISPLACED_TP_PT2_THRESHOLD="22.21"
HYBRID_DISPLACED_TP_PT3_REFERENCE="15.69"           HYBRID_DISPLACED_TP_PT3_THRESHOLD="15.31"
HYBRID_DISPLACED_TP_PT10_REFERENCE="4.44"           HYBRID_DISPLACED_TP_PT10_THRESHOLD="4.24"
HYBRID_DISPLACED_NTRK_PT2_REFERENCE="39.32"         HYBRID_DISPLACED_NTRK_PT2_THRESHOLD="39.91"
HYBRID_DISPLACED_NTRK_PT3_REFERENCE="28.68"         HYBRID_DISPLACED_NTRK_PT3_THRESHOLD="29.19"
HYBRID_DISPLACED_NTRK_PT10_REFERENCE="8.97"         HYBRID_DISPLACED_NTRK_PT10_THRESHOLD="9.25"
HYBRID_DISPLACED_FAKE_REFERENCE="22.32"             HYBRID_DISPLACED_FAKE_THRESHOLD="22.95"
HYBRID_DISPLACED_DUP_REFERENCE="8.43"               HYBRID_DISPLACED_DUP_THRESHOLD="8.85"
HYBRID_DISPLACED_Z0RES_LOWETA_REFERENCE="0.09"      HYBRID_DISPLACED_Z0RES_LOWETA_THRESHOLD="0.096"
HYBRID_DISPLACED_Z0RES_HIGHETA_REFERENCE="0.39"     HYBRID_DISPLACED_Z0RES_HIGHETA_THRESHOLD="0.416"

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
echo "=== Step 1: SUCCESS — code compiled ==="

# =============================================================================
# STEP 2 + 3: Run cmsRun and makeHists.csh for each algorithm
# =============================================================================
run_hybrid_stage() {
    local ALGO=$1
    local DATASET=$2
    local RESULTS_LABEL=$3

    echo ""
    echo "=== Step 2/3: Running algo=$ALGO ==="

    if [ ! -d "$TEST_DIR" ]; then
        echo "ERROR: Test directory not found at $TEST_DIR"
        exit 1
    fi

    cd $TEST_DIR

    JOBNAME="job_${ALGO}_${RESULTS_LABEL}_cfg.py"
    cp $CFG_FILE $JOBNAME
    sed -i "s|L1TRKALGO = 'HYBRID'|L1TRKALGO = '$ALGO'|" $JOBNAME
    grep L1TRKALGO $JOBNAME

    RESULTSDIR="$TEST_DIR/results_${ALGO}_${RESULTS_LABEL}"
    mkdir -p $RESULTSDIR

    echo "process.TFileService.fileName = cms.string('$RESULTSDIR/histos.root')" >> $JOBNAME
    echo "process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1000))" >> $JOBNAME
    echo "process.source.fileNames = cms.untracked.vstring('file:mc_dataset.root')" >> $JOBNAME
    # Comment out the line that assigns inputMC to process.source.fileNames
    # so our appended fileNames line takes effect instead.
    # (The inputMC variable definition itself is harmless and left as-is)
    sed -i 's/^\(.*process\.source\.fileNames.*=.*inputMC.*\)$/#\1/' $JOBNAME

    echo "=== Downloading dataset from CERNBox ==="
    curl -k -o mc_dataset.root $DATASET
    echo "=== Running cmsRun ==="
    cmsRun $JOBNAME
    echo "=== cmsRun: SUCCESS ==="

    echo "=== Running makeHists.csh ==="
    cd $RESULTSDIR
    tcsh ../makeHists.csh histos.root

    if [ ! -f "results.out" ]; then
        echo "FAILURE — results.out not produced by makeHists.csh"
        exit 1
    fi
    echo "=== makeHists.csh: SUCCESS ==="
    cd $TEST_DIR
}

run_hybrid_stage HYBRID $MC_DATASET_TTBAR "ttbar"

if [ "$QUICK_TEST" = "false" ]; then
    run_hybrid_stage HYBRID_NEWKF     $MC_DATASET_TTBAR "ttbar"
    run_hybrid_stage HYBRID_DISPLACED $MC_DATASET_TTBAR "ttbar"
fi

# =============================================================================
# STEP 4: Threshold validation
# =============================================================================
check_threshold() {
    local ALGO=$1
    local LABEL=$2
    local GREP_STRING=$3
    local THRESHOLD=$4
    local MODE=$5
    local METRIC_NAME=$6

    local RESULTSFILE="$TEST_DIR/results_${ALGO}_${LABEL}/results.out"

    if [ ! -f "$RESULTSFILE" ]; then
        echo "FAILURE -- $RESULTSFILE not found"
        return 1
    fi

    local VALUE=$(grep "$GREP_STRING" "$RESULTSFILE" | head -1 | cut -f2 -d= | cut -f1 -d+ | tr -d ' ' | tr -d 'cm')

    if [ -z "$VALUE" ]; then
        echo "FAILURE -- could not find '$GREP_STRING' in $RESULTSFILE"
        return 1
    fi

    local FAIL
    if [ "$MODE" = "min" ]; then
        FAIL=$(echo "$VALUE < $THRESHOLD" | bc)
    else
        FAIL=$(echo "$VALUE > $THRESHOLD" | bc)
    fi

    if (( $FAIL )); then
        echo "FAILURE -- [$ALGO/$LABEL] $METRIC_NAME = $VALUE (threshold: $THRESHOLD)"
        return 1
    else
        echo "SUCCESS -- [$ALGO/$LABEL] $METRIC_NAME = $VALUE (threshold: $THRESHOLD)"
        return 0
    fi
}

OVERALL_FAIL=0

run_all_checks() {
    local ALGO=$1
    local LABEL=$2
    local PREFIX="${ALGO}"

    check_threshold "$ALGO" "$LABEL" "efficiency for |eta| < 1.0"         "$(eval echo \$${PREFIX}_EFFI_ETA_LOW_THRESHOLD)"  "min" "effi |eta|<1.0"      || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "efficiency for 1.0 < |eta| < 1.75"  "$(eval echo \$${PREFIX}_EFFI_ETA_MID_THRESHOLD)"  "min" "effi 1.0<|eta|<1.75" || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "efficiency for 1.75 < |eta| < 2.50" "$(eval echo \$${PREFIX}_EFFI_ETA_HIGH_THRESHOLD)" "min" "effi 1.75<|eta|<2.50" || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "efficiency for pt > 2.00"            "$(eval echo \$${PREFIX}_EFFI_PT2_THRESHOLD)"      "min" "effi pt>2.00"         || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "efficiency for 2.00 < pt < 8.0"     "$(eval echo \$${PREFIX}_EFFI_PT2TO8_THRESHOLD)"   "min" "effi 2.00<pt<8.0"    || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "efficiency for pt > 8.0"             "$(eval echo \$${PREFIX}_EFFI_PT8_THRESHOLD)"      "min" "effi pt>8.0"          || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "efficiency for pt > 40.0"            "$(eval echo \$${PREFIX}_EFFI_PT40_THRESHOLD)"     "min" "effi pt>40.0"         || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "TP/event (pt > 2.00)"                "$(eval echo \$${PREFIX}_TP_PT2_THRESHOLD)"        "min" "TP/event pt>2.00"     || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "TP/event (pt > 3.0)"                 "$(eval echo \$${PREFIX}_TP_PT3_THRESHOLD)"        "min" "TP/event pt>3.0"      || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "TP/event (pt > 10.0)"                "$(eval echo \$${PREFIX}_TP_PT10_THRESHOLD)"       "min" "TP/event pt>10.0"     || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "tracks/event (pt > 2.00)"            "$(eval echo \$${PREFIX}_NTRK_PT2_THRESHOLD)"      "max" "tracks/event pt>2.00" || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "tracks/event (pt > 3.0)"             "$(eval echo \$${PREFIX}_NTRK_PT3_THRESHOLD)"      "max" "tracks/event pt>3.0"  || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "tracks/event (pt > 10.0)"            "$(eval echo \$${PREFIX}_NTRK_PT10_THRESHOLD)"     "max" "tracks/event pt>10.0" || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "Percentage fake tracks"               "$(eval echo \$${PREFIX}_FAKE_THRESHOLD)"          "max" "fake tracks %"        || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "Percentage duplicate tracks"          "$(eval echo \$${PREFIX}_DUP_THRESHOLD)"           "max" "duplicate tracks %"   || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "z0 resolution = .*0.05"              "$(eval echo \$${PREFIX}_Z0RES_LOWETA_THRESHOLD)"  "max" "z0 res |eta|=0.05"    || OVERALL_FAIL=1
    check_threshold "$ALGO" "$LABEL" "z0 resolution = .*1.95"              "$(eval echo \$${PREFIX}_Z0RES_HIGHETA_THRESHOLD)" "max" "z0 res |eta|=1.95"    || OVERALL_FAIL=1
}

echo ""
echo "=== Step 4: Validating thresholds ==="
run_all_checks HYBRID ttbar

if [ "$QUICK_TEST" = "false" ]; then
    run_all_checks HYBRID_NEWKF     ttbar
    run_all_checks HYBRID_DISPLACED ttbar
fi

echo ""
if (( $OVERALL_FAIL )); then
    echo "=== OVERALL RESULT: FAILURE -- one or more thresholds not met ==="
    exit 1
else
    echo "=== OVERALL RESULT: SUCCESS -- all thresholds passed ==="
fi
