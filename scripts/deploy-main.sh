#!/usr/bin/env bash
set -euo pipefail

APP_DIR="/home/lukeraykovitz/prod/Anime-Database-Project"
REPO_URL="git@github.com:FreezeFyre/Anime-Database-Project.git"
BRANCH="main"
BUILD_DIR="${APP_DIR}/build-release"
SERVICE_NAME="anime-database.service"

if [[ ! -d "${APP_DIR}/.git" ]]; then
	mkdir -p "$(dirname "${APP_DIR}")"
	git clone --branch "${BRANCH}" "${REPO_URL}" "${APP_DIR}"
fi

cd "${APP_DIR}"

git fetch origin "${BRANCH}"
git checkout "${BRANCH}"
git reset --hard "origin/${BRANCH}"

cmake -S . -B "${BUILD_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure

systemctl restart "${SERVICE_NAME}"
