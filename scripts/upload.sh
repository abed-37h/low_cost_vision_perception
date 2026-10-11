#!/bin/bash

while [[ $# -gt 0 ]]; do
    case "$1" in
        -b | --board)
            board="$2"
            shift 2
            ;;
        -p | --port)
            port="$2"
            shift 2
            ;;
        -c | --compile)
            compile=true
            shift
            ;;
        -m | --monitor)
            monitor=true
            shift
            ;;
        --path)
            path="$2"
            shift 2
            ;;
        -*)
            echo "Unknown flag: $1"
            exit 1
            ;;
        *)
            path="$1"
            shift
            ;;
    esac
done

case "$board" in
    xiao_esp32c3)
        fqbn="esp32:esp32:XIAO_ESP32C3"
        ;;
    arduino_mega)
        fqbn="arduino:avr:mega"
        ;;
    arduino_uno)
        fqbn="arduino:avr:uno"
        ;;
    *)
        echo "Unknown board: $board"
        exit 1
esac

if [[ ! -e "$path" ]]; then
    echo "Can't open sketch: no such file or directory: $path"
    exit 1
fi

if ! arduino-cli board list | grep -Fq "$port"; then
    echo "Error: No Arduino port detected at $port"
    exit 1
fi

if [[ "$compile" = true ]]; then
    repo_root="$(git rev-parse --show-toplevel)"
    libraries="$repo_root/libraries"
    arduino-cli compile --fqbn "$fqbn" --libraries "$libraries" --upload -p "$port" "$path"
else
    arduino-cli upload --fqbn "$fqbn" -p "$port" "$path"
fi

if [[ "$monitor" = true ]]; then
    arduino-cli monitor -p "$port"
fi