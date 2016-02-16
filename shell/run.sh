#! /bin/bash
# A script to automate the workflow of
# make
# run some command with some environment variables preset
# make clean

##################################### VARS #####################################

readonly PROGNAME=$(basename $0)
# associative array (requires bash > version 4 )
declare -A ENV_VARS=( ["SOME_ENV_VAR"]="10000" \
                      ["SOME_OTHER_ENV_VAR_DISABLE"]="1" \
                      ["LD_LIBRARY_PATH"]="$LD_LIBRARY_PATH:." )

readonly DEF_LINUX_PRELOAD=./mylib.so
readonly DEF_DARWIN_PRELOAD=./mylib_in_osx.dylib
readonly MAKE_DEBUG_CMD="make -B debug"
readonly MAKE_CMD="make -B all"
readonly MAKE_CLEAN_CMD="make clean"

################################################################################

usage() {
    echo "Usage: call with '$PROGNAME [OPTION]'"
    echo "e.g. $PROGNAME -p -c \"./test\" "
    echo "Flags:"
    echo "      -h print help"
    echo "      -e [full path of app to be tested including its arguments in double quotes]"
    echo "      -x run bash in debug mode"
    echo "      -l [path to library that will be preloaded]"
    echo "      -r to rebuild local project before running command"
    echo "      -c to do make clean after running command"
    echo "      -p whether to preload a library or not"
    echo "      -d whether to start gdb with the given command instead"
    echo "      -v verbose output in shell"
}

parse_args() {
    while getopts "pvdrchxl:e:" OPTION
    do
         case $OPTION in
         v)  set -x
             ;;
         p)  readonly PRELOAD_LIB=1
             ;;
         r)  readonly REBUILD=1
             ;;
         c)  readonly CLEAN_AFTER=1
             ;;
         h)  usage
             exit 0
             ;;
         d)  readonly GDB_ENABLED=1
             ;;
         x)  readonly DEBUG=1
             ;;
         l)  PRELOAD_LIB_PATH=$OPTARG
             ;;
         e)  COMMAND=$OPTARG
             ;;
         \?) usage
             exit 1
             ;;
        esac
    done

    [[ -z "$COMMAND" ]] && \
        usage && \
        exit;

    return 0
}

main() {

    parse_args "${@}"

    local UNAMESTR=$(uname)

    # Add LD_PRELOAD if instructed to
    [[ ! -z $PRELOAD_LIB ]] && \
        if [ "$UNAMESTR" == 'Linux' ]; then
            ENV_VARS["LD_PRELOAD"]="${PRELOAD_LIB_PATH:-$DEF_LINUX_PRELOAD}"
        elif [ "$UNAMESTR" == 'Darwin' ]; then
            ENV_VARS["DYLD_INSERT_LIBRARIES"]="${PRELOAD_LIB_PATH:-$DEF_DARWIN_PRELOAD}"
        fi

    # Concatenate all environment variables in name=value form
    # as well as gdb command + all env vars in two long strings
    ENV_PRE_STRING=""
    GDB_PRE_STRING="gdb"
    for env_var in "${!ENV_VARS[@]}"; do
        ENV_PRE_STRING="$ENV_PRE_STRING $env_var=${ENV_VARS["$env_var"]}" ;
        GDB_PRE_STRING="$GDB_PRE_STRING -ex \"set environment $env_var=${ENV_VARS["$env_var"]}\"" ;
    done

    if [ ! -z $REBUILD ]; then
        if [ ! -z $DEBUG ]; then
            $MAKE_DEBUG_CMD
        else
            $MAKE_CMD
        fi
    fi

    if [ ! -z $GDB_ENABLED ]; then
        # run gdb and include the environment variables in it as arguments to -ex
        eval "$GDB_PRE_STRING -ex \"file ${COMMAND}\""
    else
        # run command and prepend all environment variables before it
        eval "$ENV_PRE_STRING $COMMAND"
    fi

    [[ ! -z $CLEAN_AFTER ]] && \
        $MAKE_CLEAN_CMD

    # clear LD_PRELOAD
    unset LD_PRELOAD
}

main "${@}"