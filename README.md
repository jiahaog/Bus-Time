# Bus Time

![Header](https://raw.githubusercontent.com/skewedlines/Bus-Time/master/workspace/Resources/MarketingHeader-color.png?token=AGyEPTM8xQ5ZouN-f07WwwJ7X-3t3sMVks5V1WMVwA%3D%3D)

## Introduction

![Screenshot2](https://raw.githubusercontent.com/skewedlines/Bus-Time/master/workspace/Resources/screenshots/screenshot-2-color.png?token=AGyEPVk-8LAkrEpE4rAyrmWVYx1lcr1Lks5V1WMXwA%3D%3D)

![Screenshot1](https://raw.githubusercontent.com/skewedlines/Bus-Time/master/workspace/Resources/screenshots/screenshot-1-color.png?token=AGyEPXViwQA9JtV96Y4MSO3ZLllIwFhVks5V1WMZwA%3D%3D)

A [Pebble Time](http://getpebble.com) app to retrieve bus arrival timings from bus stops nearby.

## Dependencies

- [Pebble-SDK](http://developer.getpebble.com/sdk/install/mac/)
- [Browserify](http://browserify.org)

## Build

```bash
$ git clone git@github.com:skewedlines/Bus-Time.git
$ cd Bus-Time

# Browserify compiles the Javascript modules and code into a single file
$ browserify workspace/JSWorkspace/src/main.js -o src/js/pebble-js-app.js 

$ pebble build 
$ pebble install --logs --emulator basalt
```
## Todo

- Add alert notifications for bus arrival
- Pebble Timeline support
- More countries
- Transfer bus stop database to web server