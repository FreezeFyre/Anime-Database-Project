#!/usr/bin/env bash
set -euo pipefail

APP_DIR="/home/lukeraykovitz/prod/Anime-Database-Project"
REPO_URL="git@github.com:FreezeFyre/Anime-Database-Project.git"
BRANCH="main"
BUILD_DIR="${APP_DIR}/build-release"
SERVICE_NAME="anime-database.service"
DEPLOY_USER="lukeraykovitz"

run_as_deploy_user() {
	runuser -u "${DEPLOY_USER}" -- "$@"
}

if [[ ! -d "${APP_DIR}/.git" ]]; then
	mkdir -p "$(dirname "${APP_DIR}")"
	chown "${DEPLOY_USER}:${DEPLOY_USER}" "$(dirname "${APP_DIR}")"
	run_as_deploy_user git clone --branch "${BRANCH}" "${REPO_URL}" "${APP_DIR}"
fi

run_as_deploy_user git -C "${APP_DIR}" fetch origin "${BRANCH}"
run_as_deploy_user git -C "${APP_DIR}" checkout "${BRANCH}"
run_as_deploy_user git -C "${APP_DIR}" reset --hard "origin/${BRANCH}"

run_as_deploy_user cmake -S "${APP_DIR}" -B "${BUILD_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=Release
run_as_deploy_user cmake --build "${BUILD_DIR}"
run_as_deploy_user ctest --test-dir "${BUILD_DIR}" --output-on-failure

systemctl restart "${SERVICE_NAME}"
