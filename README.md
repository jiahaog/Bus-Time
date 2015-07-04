# Bus Time

## Introduction

A [Pebble Time](http://getpebble.com) app to retrieve bus arrival timings from stops nearby, and featuring an alert system for when buses arrive.

## Dependencies

- [Pebble-SDK](http://developer.getpebble.com/sdk/install/mac/)
- [Browserify](http://browserify.org)

## Building for Emulator

Clone the repo first

```bash
$ cd Bus-Time

# Browserify compiles the Javascript modules and code into a single file
$ browserify workspace/JSWorkspace/src/main.js -o src/js/pebble-js-app.js 

$ pebble build 
$ pebble install --logs --emulator basalt
```
