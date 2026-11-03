#!/bin/bash
set -e
cd "$(dirname "$0")"
make
exec ./page_simulator
