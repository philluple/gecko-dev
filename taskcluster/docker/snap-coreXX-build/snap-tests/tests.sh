#!/bin/bash

set -ex

pwd

SUITE=${1:-basic}
SUITE=${SUITE//--/}

export ARTIFACT_DIR=$TASKCLUSTER_ROOT_DIR/builds/worker/artifacts/
mkdir -p "$ARTIFACT_DIR"

# There's a bug in snapd ~2.60.3-2.61 that will make "snap refresh" fail
# https://bugzilla.mozilla.org/show_bug.cgi?id=1873359
# https://bugs.launchpad.net/snapd/+bug/2048104/comments/13
# 
# So we retrieve the version and we will allow the first "snap refresh" to
# fail for versions < 2.61.1
SNAP_VERSION=$(snap info snapd --color=never --unicode=never |grep "installed:" | awk '{ print $2 }')
SNAP_MAJOR=$(echo "${SNAP_VERSION}" | cut -d'.' -f1)
SNAP_MINOR=$(echo "${SNAP_VERSION}" | cut -d'.' -f2)
SNAP_RELEASE=$(echo "${SNAP_VERSION}" | cut -d'.' -f3)

REFRESH_CAN_FAIL=true
if [ "${SNAP_MAJOR}" -ge 2 ]; then
  if [ "${SNAP_MAJOR}" -gt 2 ]; then
    REFRESH_CAN_FAIL=false
  else
    if [ "${SNAP_MINOR}" -ge 61 ]; then
      if [ "${SNAP_MINOR}" -gt 61 ]; then
        REFRESH_CAN_FAIL=false
      else
        if [ "${SNAP_RELEASE}" -gt 0 ]; then
          REFRESH_CAN_FAIL=false
        fi
      fi
    fi
  fi
fi

if [ "${REFRESH_CAN_FAIL}" = "true" ]; then
  sudo snap refresh || true
else
  sudo snap refresh
fi;

sudo snap refresh --hold=24h firefox

snap connections firefox

# Collect existing connections from Snap store
# Replace "firefox" snap name by SNAP_FIREFOX_INSTANCE
(snap connections firefox | grep -v "Interface" | while read -r line; do INT=$(echo "${line}" | awk '{ print $1 }'); CONN=$(echo "${line}" | awk '{ print $2 }' | sed -e 's/firefox:/SNAP_FIREFOX_INSTANCE:/g'); PLUG=$(echo "${line}" | awk '{ print $3 }' | sed -e 's/firefox:/SNAP_FIREFOX_INSTANCE:/g'); echo "${INT};${CONN};${PLUG}"; done) > firefox.connections.txt
cat firefox.connections.txt

while true; do
  if snap changes 2>&1 | grep -E '(Doing|Undoing|Do\s|restarting)'; then
    echo wait; sleep 0.5
  else
    break
  fi
done

export TEST_SNAP_INSTANCE=${TEST_SNAP_INSTANCE:-firefox}

sudo snap install --name "${TEST_SNAP_INSTANCE}" --dangerous ./firefox.snap

snap connections "${TEST_SNAP_INSTANCE}"

(snap connections "${TEST_SNAP_INSTANCE}" | grep -v "Interface" | while read -r line; do INT=$(echo "${line}" | awk '{ print $1 }'); CONN=$(echo "${line}" | awk '{ print $2 }' | sed -e "s/${TEST_SNAP_INSTANCE}:/SNAP_FIREFOX_INSTANCE:/g"); PLUG=$(echo "${line}" | awk '{ print $3 }' | sed -e "s/${TEST_SNAP_INSTANCE}:/SNAP_FIREFOX_INSTANCE:/g"); echo "${INT};${CONN};${PLUG}"; done) > instance.connections.txt
cat instance.connections.txt

# shellcheck disable=SC2013
for missing_connection in $(grep ';-$' instance.connections.txt);
do
  echo "MISSING ${missing_connection}"
  KEY="$(echo "$missing_connection" | cut -d';' -f1,2)"
  STORE=$(grep "$KEY;" firefox.connections.txt | cut -d';' -f3)
  PLUG="$STORE"
  if [ "${STORE}" = "-" ] || [ -z "${STORE}" ]; then
    echo "NO PREVIOUS STORE CONNECTION FOUND FOR '${KEY}' CHECKING FOR NEW LOCAL"
    LOCAL=$(grep "$KEY;" local.connections.txt | cut -d';' -f3)
    if [ "${LOCAL}" = "-" ] || [ -z "${LOCAL}" ]; then
      echo "NO PREVIOUS STORE NOR LOCAL CONNECTION FOUND FOR '${KEY}'. ABORTING"
      continue
    else
      PLUG="$LOCAL"
    fi
  fi
  CONNECTOR="$(echo "${missing_connection}" | cut -d';' -f2 | sed -e "s/SNAP_FIREFOX_INSTANCE/$TEST_SNAP_INSTANCE/g")"
  sudo snap connect "${CONNECTOR}" "${PLUG}"
done;

snap connections "${TEST_SNAP_INSTANCE}"

RUNTIME_VERSION=$(snap run firefox --version | awk '{ print $3 }')

mkdir -p ~/.config/pip/ && cat > ~/.config/pip/pip.conf <<EOF
[global]
break-system-packages = true
EOF

SNAP_PROFILE_PATH="$HOME/snap/${TEST_SNAP_INSTANCE}/common/.mozilla/firefox/"
if [ ! -d "${SNAP_PROFILE_PATH}" ]; then
  mkdir -p "${SNAP_PROFILE_PATH}"
fi

python3 -m pip install --user -r requirements.txt

# Required otherwise copy/paste does not work
# Bug 1878643
export TEST_NO_HEADLESS=1

if [ -n "${MOZ_LOG}" ]; then
  export MOZ_LOG_FILE="${ARTIFACT_DIR}/gecko-log"
fi

RECORD_SCREEN_PID=0
if [ "${TEST_RECORD_SCREEN}" = "true" ]; then
  python3 record.py &
  RECORD_SCREEN_PID=$!
  echo "Recording with PID ${RECORD_SCREEN_PID}"

  trap 'echo [EXIT] Stopping screen recording from PID ${RECORD_SCREEN_PID} && kill ${RECORD_SCREEN_PID}' EXIT
  trap 'echo [ERR] Stopping screen recording from PID ${RECORD_SCREEN_PID} && kill ${RECORD_SCREEN_PID}' ERR
fi;

if [ "${SUITE}" = "basic" ]; then
  sed -e "s/#RUNTIME_VERSION#/${RUNTIME_VERSION}/#" < basic_tests/expectations.json.in > basic_tests/expectations.json
  python3 basic_tests.py basic_tests/expectations.json
else
  python3 "${SUITE}"_tests.py
fi;
