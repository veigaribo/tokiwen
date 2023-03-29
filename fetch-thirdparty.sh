#!/usr/bin/env bash

# This script clones a Git repository, puts selected files in a given directory, and outputs
# an entry for `thirdparty/README.md` to stdout.
#
# There is a function for every library that defines exactly what to do in each case.
# Call this by referring to the function name in the first argument and the directory in the
# second.

TMP_DIR="/tmp/tokiwen/thirdparty"
mkdir -p "$TMP_DIR"

function on_error {
    rm -rf "$TMP_DIR"
}

trap on_error ERR

if test $? -ne 0; then
    >&2 echo "Could not set ERR trap. If this script fails, $TMP_DIR may be left dirty."
fi

set -e

REPORT_FILE="$TMP_DIR/reporting"
ORIGINAL_WD=`pwd`

echo "" > "$REPORT_FILE"

destination_dir_base=""
destination_dir=""

function git_default_branch {
    git rev-parse --abbrev-ref origin/HEAD | sed -e "s|origin/||"
}

# Steps.
# These should be called in order so as to correctly build the credits file.

function repo {
    local name="$1"
    local url="$2"
    local args="$3"

    if ! test -d "$TMP_DIR/$name"; then
        git clone $args "$url" "$TMP_DIR/$name" 1> /dev/null
        cd "$TMP_DIR/$name"
    else
        cd "$TMP_DIR/$name"
        git pull origin "$(git_default_branch)" 1> /dev/null
    fi

    echo "# $name" >> "$REPORT_FILE"
    echo "- Upstream: <$url>" >> "$REPORT_FILE"

    destination_dir="$destination_dir_base/$name"
    rm -rf "$destination_dir/"* # Will not remove hidden stuff
}

function checkout_tag {
    local tag="$1"

    git fetch origin --tags
    git switch --detach "$tag"
    local commit=`git rev-parse HEAD`

    echo "- Version: Tag $tag (Commit $commit)" >> "$REPORT_FILE"
}

function checkout_ref {
    local ref="$1"

    git switch --detach "$ref"
    local commit=`git rev-parse HEAD`

    echo "- Version: Commit $commit" >> "$REPORT_FILE"
}

function set_license_name {
    local license_name="$1"
    echo "- License: $license_name" >> "$REPORT_FILE"
}

function grab {
    local globs=("$@")
    local files=`git ls-files --exclude-standard ${globs[@]}`

    IFS=$'\n'
    for file in $files; do
        local local_dir=`dirname "$file"`
        local dest_dir="$destination_dir/$local_dir"

        mkdir -p $dest_dir

        cp "$file" "$dest_dir"
    done
    unset IFS

    local extracted="\`${globs[0]}\`"
    for glob in "${globs[@]:1}"; do
        extracted="$extracted, \`$glob\`"
    done

    echo -n "- Extracted: $extracted" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
}

# Libraries.

function bandit {
    repo "bandit" "https://github.com/banditcpp/bandit.git"
    checkout_ref "HEAD"
    set_license_name "MIT"

    grab "bandit/" ":!:bandit/assertion_frameworks/snowhouse"
    grab "LICENSE.txt"

    echo "" >> "$REPORT_FILE"
    snowhouse
}

function snowhouse {
    if ! test -d "$destination_dir_base/bandit"; then
      echo >&2 "\`bandit\` should be installed before snowhouse."
      exit 1
    fi

    repo "snowhouse" "https://github.com/banditcpp/snowhouse.git"
    checkout_tag "v5.0.0"
    set_license_name "Boost Software License 1.0"

    grab "*.h" ":!:example/"
    grab "LICENSE_1_0.txt"

    # create the directory since it could not have been copied
    mkdir -p "$destination_dir_base/bandit/bandit/assertion_frameworks"
    ln -s --force --no-target-directory "$destination_dir/include/snowhouse" "$destination_dir_base/bandit/bandit/assertion_frameworks/snowhouse"
}

function RE-flex {
    repo "RE-flex" "https://github.com/Genivia/RE-flex.git"
    checkout_tag "v3.3.1"
    set_license_name "BSD 3-Clause"

    grab "unicode/" "lib/" "src/" "include/" "fuzzy/" ":!:**/Make*" ":!:**/README*" ":!:**/*.txt"
    grab "LICENSE.txt"
}

# Execute.

library="$1"
destination_dir="$2"

if test -z "$library"; then
    >&2 echo "Missing library (first) parameter."
    >&2 echo "Missing destination directory (second) parameter. (Probably 'thirdparty')"
    exit 1
fi

if test -z "$destination_dir"; then
    >&2 echo "Missing destination directory (second) parameter. (Probably 'thirdparty')"
    exit 1
fi

if test ! -d "$destination_dir"; then
    >&2 echo "Destination directory '$destination_dir' is not a directory."
    >&2 echo "Note that it should not include the library name."
    exit 1
fi

# Set output directory for other commands
destination_dir_base="$ORIGINAL_WD/$destination_dir"

# Run function with the given name
"$1"

# Send report to stdout
cat "$REPORT_FILE"

# Cleanup (probably unnecessary)
# rm -rf "$TMP_DIR"
