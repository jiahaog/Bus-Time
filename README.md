# Bus Time

![Header](https://raw.githubusercontent.com/jiahaog/Bus-Time/master/workspace/Resources/MarketingHeader-color.png?token=AGyEPabwGm5sv2vZbO7PzKGcqCwmg1tHks5WwWd7wA%3D%3D)

## Introduction

![Screenshot2](https://raw.githubusercontent.com/jiahaog/Bus-Time/master/workspace/Resources/screenshots/screenshot-2-color.png?token=AGyEPRPevx6IjPuuXfaA7m4wtq8DY8TBks5WwWeRwA%3D%3D)

![Screenshot1](https://raw.githubusercontent.com/jiahaog/Bus-Time/master/workspace/Resources/screenshots/screenshot-1-color.png?token=AGyEPfWOyt6LR9GQXk9T1jFtnINjgb8uks5WwWeiwA%3D%3D)

A [Pebble Time](http://getpebble.com) app to retrieve bus arrival timings from bus stops nearby.

## Dependencies

- [Pebble-SDK](http://developer.getpebble.com/sdk/install/mac/)
- [Browserify](http://browserify.org)

## Build

```bash
$ git clone git@github.com:jiahaog/Bus-Time.git
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
