#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/PhotonDetector/build"
EXECUTABLE="${BUILD_DIR}/PhotonDetector"

REPS=5
START_REP=1
BEAM_ON=10000
RUN_JOBS=0
APPEND=0
GEOMETRY="fully"
OUT_ROOT="${SCRIPT_DIR}/rn222_repetition_runs"
RUN_LABEL="$(date +%Y%m%d_%H%M%S)"

usage() {
  cat <<EOF
Usage: $(basename "$0") [options]

Generate repeated Rn222 configuration macros and optional run commands.

Options:
  --reps N          Number of repetitions per case. Default: ${REPS}
  --start-rep N     First repetition index to generate. Default: ${START_REP}
  --beam-on N       /run/beamOn value. Default: ${BEAM_ON}
  --geometry NAME    Glue source geometry: fully or bottom50um. Default: ${GEOMETRY}
  --out-root DIR    Parent output directory. Default: ${OUT_ROOT}
  --label LABEL     Run folder label. Default: timestamp
  --append          Append to existing generated_files_log.tsv and run_all.sh.
  --run             Execute jobs after generating macros.
  --help            Show this help.

Generated layout:
  OUT_ROOT/LABEL/
    macros/*.mac
    outputs/*_photon_boundaries.csv
    outputs/*_Rn222_exit_glue_events.csv
    logs/*.log
    generated_files_log.tsv
    run_all.sh
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --reps)
      REPS="$2"
      shift 2
      ;;
    --start-rep)
      START_REP="$2"
      shift 2
      ;;
    --beam-on)
      BEAM_ON="$2"
      shift 2
      ;;
    --geometry)
      GEOMETRY="$2"
      shift 2
      ;;
    --out-root)
      OUT_ROOT="$2"
      shift 2
      ;;
    --label)
      RUN_LABEL="$2"
      shift 2
      ;;
    --run)
      RUN_JOBS=1
      shift
      ;;
    --append)
      APPEND=1
      shift
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

if ! [[ "${REPS}" =~ ^[0-9]+$ ]] || (( REPS < 1 )); then
  echo "--reps must be a positive integer: ${REPS}" >&2
  exit 1
fi

if ! [[ "${START_REP}" =~ ^[0-9]+$ ]] || (( START_REP < 1 )); then
  echo "--start-rep must be a positive integer: ${START_REP}" >&2
  exit 1
fi

if [[ ! -x "${EXECUTABLE}" ]]; then
  echo "PhotonDetector executable not found or not executable: ${EXECUTABLE}" >&2
  exit 1
fi

END_REP=$((START_REP + REPS - 1))

case "${GEOMETRY}" in
  fully)
    OUTPUT_PREFIX="2mm_2mm"
    GLUE_HALFX="20 mm"
    GLUE_HALFY="1 mm"
    GLUE_HALFZ="20 mm"
    GLUE_CENTER="0 101 0 mm"
    ;;
  bottom50um)
    OUTPUT_PREFIX="2mm_50um_bottom"
    GLUE_HALFX="20 mm"
    GLUE_HALFY="0.025 mm"
    GLUE_HALFZ="20 mm"
    GLUE_CENTER="0 100.025 0 mm"
    ;;
  *)
    echo "Unsupported --geometry: ${GEOMETRY}. Expected fully or bottom50um." >&2
    exit 1
    ;;
esac

RUN_DIR="${OUT_ROOT}/${RUN_LABEL}"
MACRO_DIR="${RUN_DIR}/macros"
OUTPUT_DIR="${RUN_DIR}/outputs"
LOG_DIR="${RUN_DIR}/logs"
GENERATED_FILES_LOG="${RUN_DIR}/generated_files_log.tsv"
RUN_ALL="${RUN_DIR}/run_all.sh"

mkdir -p "${MACRO_DIR}" "${OUTPUT_DIR}" "${LOG_DIR}"

format_beam_label() {
  local n="$1"
  if (( n % 1000000 == 0 )); then
    echo "$((n / 1000000))M"
  elif (( n % 1000 == 0 )); then
    echo "$((n / 1000))k"
  else
    echo "$n"
  fi
}

BEAM_LABEL="$(format_beam_label "${BEAM_ON}")"

write_header() {
  local macro="$1"
  local seed1="$2"
  local seed2="$3"
  {
    echo "/run/numberOfThreads 1"
    echo "/random/setSeeds ${seed1} ${seed2}"
    echo
    echo "/det/set_yGlue 1000 um"
    echo "/det/set_detectorDistance 20 cm"
    echo "/det/set_xviScintillatorThickness 1 mm"
    echo
    echo "/run/initialize"
    echo
  } > "${macro}"
}

append_rn222_source() {
  local macro="$1"
  local intensity="$2"
  local halfx="$3"
  local halfy="$4"
  local halfz="$5"
  local center="$6"
  local add_source="${7:-0}"

  {
    if [[ "${add_source}" == "1" ]]; then
      echo
      echo "/gps/source/add ${intensity}"
    else
      echo "/gps/source/intensity ${intensity}"
    fi
    echo "/gps/pos/type Volume"
    echo "/gps/pos/shape Para"
    echo "/gps/pos/halfx ${halfx}"
    echo "/gps/pos/halfy ${halfy}"
    echo "/gps/pos/halfz ${halfz}"
    echo "/gps/pos/centre ${center}"
    echo
    echo "/gps/particle ion"
    echo "/gps/ion 86 222 0 0"
    echo "/gps/ene/type Mono"
    echo "/gps/energy 0 MeV"
    echo
  } >> "${macro}"
}

append_footer() {
  local macro="$1"
  {
    echo "/run/printProgress 1000"
    echo "/run/beamOn ${BEAM_ON}"
  } >> "${macro}"
}

write_case_macro() {
  local macro="$1"
  local seed1="$2"
  local seed2="$3"
  local case_key="$4"

  write_header "${macro}" "${seed1}" "${seed2}"

  case "${case_key}" in
    baseline)
      append_rn222_source "${macro}" "1.0" "${GLUE_HALFX}" "${GLUE_HALFY}" "${GLUE_HALFZ}" "${GLUE_CENTER}"
      ;;
    diff10)
      append_rn222_source "${macro}" "0.9" "${GLUE_HALFX}" "${GLUE_HALFY}" "${GLUE_HALFZ}" "${GLUE_CENTER}"
      append_rn222_source "${macro}" "0.1" "10 cm" "10 cm" "75 cm" "0 0 0 mm" 1
      ;;
    diff50)
      append_rn222_source "${macro}" "0.5" "${GLUE_HALFX}" "${GLUE_HALFY}" "${GLUE_HALFZ}" "${GLUE_CENTER}"
      append_rn222_source "${macro}" "0.5" "10 cm" "10 cm" "75 cm" "0 0 0 mm" 1
      ;;
	    diff90)
	      append_rn222_source "${macro}" "0.1" "${GLUE_HALFX}" "${GLUE_HALFY}" "${GLUE_HALFZ}" "${GLUE_CENTER}"
	      append_rn222_source "${macro}" "0.9" "10 cm" "10 cm" "75 cm" "0 0 0 mm" 1
	      ;;
	    loc20)
	      append_rn222_source "${macro}" "0.8" "${GLUE_HALFX}" "${GLUE_HALFY}" "${GLUE_HALFZ}" "${GLUE_CENTER}"
	      append_rn222_source "${macro}" "0.2" "2.5 cm" "2.5 cm" "2.5 cm" "0 0 50 cm" 1
	      ;;
	    loc40)
	      append_rn222_source "${macro}" "0.6" "${GLUE_HALFX}" "${GLUE_HALFY}" "${GLUE_HALFZ}" "${GLUE_CENTER}"
	      append_rn222_source "${macro}" "0.4" "2.5 cm" "2.5 cm" "2.5 cm" "0 0 50 cm" 1
      ;;
    loc60)
      append_rn222_source "${macro}" "0.4" "${GLUE_HALFX}" "${GLUE_HALFY}" "${GLUE_HALFZ}" "${GLUE_CENTER}"
      append_rn222_source "${macro}" "0.6" "2.5 cm" "2.5 cm" "2.5 cm" "0 0 50 cm" 1
      ;;
    *)
      echo "Unknown case: ${case_key}" >&2
      exit 1
      ;;
  esac

  append_footer "${macro}"
}

case_label() {
  case "$1" in
    baseline) echo "${OUTPUT_PREFIX}_${BEAM_LABEL}_baseline" ;;
    diff10) echo "${OUTPUT_PREFIX}_${BEAM_LABEL}_10diffRn222" ;;
	    diff50) echo "${OUTPUT_PREFIX}_${BEAM_LABEL}_50diffRn222" ;;
	    diff90) echo "${OUTPUT_PREFIX}_${BEAM_LABEL}_90diffRn222" ;;
	    loc20) echo "${OUTPUT_PREFIX}_${BEAM_LABEL}_20_Rn222_loc" ;;
	    loc40) echo "${OUTPUT_PREFIX}_${BEAM_LABEL}_40_Rn222_loc" ;;
	    loc60) echo "${OUTPUT_PREFIX}_${BEAM_LABEL}_60_Rn222_loc" ;;
    *) echo "$1" ;;
  esac
}

CASES=(baseline diff10 diff50 diff90 loc40 loc60 loc20)

if [[ "${APPEND}" == "1" && -f "${GENERATED_FILES_LOG}" ]]; then
  :
else
  {
    echo -e "case\trep\tseed1\tseed2\tmacro\toutput_stem\tlog"
  } > "${GENERATED_FILES_LOG}"
fi

if [[ "${APPEND}" == "1" && -f "${RUN_ALL}" ]]; then
  :
else
  {
    echo "#!/usr/bin/env bash"
    echo "set -euo pipefail"
    echo
  } > "${RUN_ALL}"
fi

case_index=0
for case_key in "${CASES[@]}"; do
  case_index=$((case_index + 1))
  label="$(case_label "${case_key}")"
  for rep in $(seq "${START_REP}" "${END_REP}"); do
    rep_label="$(printf "rep%02d" "${rep}")"
    seed1=$((870000 + case_index * 1000 + rep * 2 - 1))
    seed2=$((870000 + case_index * 1000 + rep * 2))
    macro="${MACRO_DIR}/${label}_${rep_label}.mac"
    output_stem="${OUTPUT_DIR}/${label}_${rep_label}"
    log="${LOG_DIR}/${label}_${rep_label}.log"

    if [[ "${APPEND}" == "1" && -f "${GENERATED_FILES_LOG}" ]] && \
      awk -F '\t' -v case_key="${case_key}" -v rep="${rep}" \
        'NR > 1 && $1 == case_key && $2 == rep { found = 1 } END { exit found ? 0 : 1 }' \
        "${GENERATED_FILES_LOG}"; then
      echo "Refusing to append duplicate entry for case=${case_key}, rep=${rep} in ${GENERATED_FILES_LOG}" >&2
      exit 1
    fi

    write_case_macro "${macro}" "${seed1}" "${seed2}" "${case_key}"

    printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\n" \
      "${case_key}" "${rep}" "${seed1}" "${seed2}" "${macro}" "${output_stem}" "${log}" >> "${GENERATED_FILES_LOG}"

    printf 'cd %q && %q -mac %q -o %q > %q 2>&1\n' \
      "${BUILD_DIR}" "${EXECUTABLE}" "${macro}" "${output_stem}" "${log}" >> "${RUN_ALL}"
  done
done

chmod +x "${RUN_ALL}"

echo "Generated ${#CASES[@]} cases x ${REPS} repetitions (rep ${START_REP}-${END_REP}) in: ${RUN_DIR}"
echo "Generated files log: ${GENERATED_FILES_LOG}"
echo "Runner:   ${RUN_ALL}"

if [[ "${RUN_JOBS}" == "1" ]]; then
  echo "Running generated jobs..."
  "${RUN_ALL}"
fi
