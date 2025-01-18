#!/bin/bash

# Define functions for each target
build-release() {
  cmake -S . -B $PROJECT_DIR/build_$TARGET -DCMAKE_BUILD_TYPE=Release 
  cmake --build $PROJECT_DIR/build_$TARGET
}

build-debug() {
  cmake -S . -B $PROJECT_DIR/build_$TARGET -DCMAKE_BUILD_TYPE=Debug 
  cmake --build $PROJECT_DIR/build_$TARGET
}

run() {
  if [[ $LOG_TO_FILE != "" ]]; then
    echo "Logging to directory $PROJECT_DIR/build_$TARGET/log"
    if [ ! -d "$PROJECT_DIR/build_$TARGET/log" ]; then
      mkdir $PROJECT_DIR/build_$TARGET/log
    fi
    $PROJECT_DIR/build_$TARGET/$PROJECT_NAME --log_dir="$PROJECT_DIR/build_$TARGET/log"
  else 
    $PROJECT_DIR/build_$TARGET/$PROJECT_NAME
  fi
}

test() {
  cd $PROJECT_DIR/build_$TARGET && ctest --rerun-failed --output-on-failure
}

clean() {
  echo "Deleting $PROJECT_DIR/build_$TARGET"
  rm -rf $PROJECT_DIR/build_$TARGET
}

PROJECT_DIR=$(pwd)
PROJECT_NAME=$(basename "$PROJECT_DIR")

# Available commands
COMMANDS=(build run test clean)
# Function to check if a command exists
has_command() {
  local COMMAND="$1"
  shift
  for arg; do
    if [ "$arg" == "$COMMAND" ]; then
      return 0
    fi
  done
  return 1
}

# Define valid target options
declare -a VALID_TARGETS=("debug" "release")

# Set default target if not provided
TARGET="debug"  # Change this to the default target if needed

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --target)
      if [[ " ${VALID_TARGETS[@]} " =~ " $2 " ]]; then
        TARGET="$2"
        shift 2  # Move past "--target" and its argument
      else
        echo "Error: Invalid target '$2'. Valid targets: ${VALID_TARGETS[@]}"
        exit 1
      fi
      ;;
    --log)
      LOG_TO_FILE=1
      shift 1  # Move past "--log" and its argument
      ;;
    *)
      # Assume command
      break
      ;;
  esac
done

# Check if at least one command is provided
if [ -z "$1" ]; then
  echo "Error: No command provided. Available commands: build, test, run, clean"
  exit 1
fi

# Loop through arguments and execute corresponding functions
for COMMAND; do
  # Check if the command is valid
  if ! has_command "$COMMAND" "${COMMANDS[@]}"; then
      echo "Error: Unknown command '$COMMAND'."
      exit 1
  fi

  # Call the appropriate function based on the command and target
  case $COMMAND in
  build)
      build-$TARGET
      ;;
  run)
      run
      ;;
  test)
      test
      ;;
  clean)
      clean
      ;;
  esac
    
    # Check the exit status of the last executed command
  COMMAND_RESULT=$?
  if [ $COMMAND_RESULT -ne 0 ]; then
      echo "$COMMAND failed. Exiting."
      exit $COMMAND_RESULT
  fi
  
  # Always return to root project dir
  cd $PROJECT_DIR
done

echo "Runner done without errors"